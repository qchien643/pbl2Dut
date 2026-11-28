# Thiết kế service và luồng xử lý

Tài liệu giải thích hàm làm gì, gọi thành phần nào và thực hiện mã BR nào.
Chính sách đầy đủ nằm trong [Business Rules Catalog](../requirements/business-rules.md);
không mặc định mọi hàm service đều có nghiệp vụ riêng.

## 1. CatalogService

Luồng minh họa: [sửa/xóa phòng](../diagrams/room-change-flow/README.md).

Nguồn: [header](../../src/services/CatalogService.h),
[implementation](../../src/services/CatalogService.cpp).

| Hàm | Xử lý và quy tắc liên quan |
|---|---|
| `addFilm()` | Lấy ID, tạo candidate, kiểm tra tên qua `requireUniqueFilmTitle()`, gọi `insert()`. BR-C04. |
| `updateFilm()` | Tra phim hiện tại, kiểm tra thay đổi thời lượng và tên rồi gọi `update()`. BR-C01/C04. |
| `addRoom()` | Tạo candidate, kiểm tra tên, sinh ghế, gọi `IRoomStorage::add()`. BR-C02/C05. |
| `updateRoom()` | Gọi `requireRoomUnused()` trước, sau đó kiểm tra tên, sinh lại ghế và gọi `IRoomStorage::update()`. BR-C02/C03/C05. |
| `eraseFilm()` | Tra phim, kiểm tra tham chiếu qua `filmReferenced()`, gọi repository xóa. BR-D01. |
| `eraseRoom()` | Gọi `requireRoomUnused()`, lấy danh sách ghế, xóa ghế thuộc phòng rồi xóa phòng. BR-D02/D03. |
| `film()`, `room()` | Kiểm tra ID dương, gọi `findById()`, báo lỗi nếu thiếu; chỉ tra cứu có xử lý lỗi. |
| `films()` | Chuỗi rỗng lấy tất cả qua `list()`; nội dung khác gọi `IFilmRepository::searchTitle()` trên cùng kho rồi copy kết quả. Index và xử lý chữ nằm dưới service; không thêm điều kiện nghiệp vụ. |
| `rooms()` | Lấy danh sách từ repository rồi copy vào kết quả; không có nghiệp vụ bổ sung. |
| `seatMap()` | Tra phòng rồi gọi `makeRoomSeatMap()` kiểm tra sơ đồ đầy đủ theo BR-C02; chưa xác định ghế đã bán theo suất. |
| `makeRoomSeatMap()` | Hàm static dùng chung: lấy ghế thuộc phòng, yêu cầu đủ số lượng rồi gọi helper dựng ma trận. Không tạo repository hay giữ trạng thái. |

Hàm nội bộ:

- `requireUniqueFilmTitle()`/`requireUniqueRoomName()`: lấy danh sách qua
  `IRepository<T>::list()`, so tên đã được constructor xử lý và bỏ qua ID của
  candidate. Chưa ghi vào kho nên trùng tên không làm đổi bản ghi, ID hay dirty.
- `filmReferenced()`/`requireRoomUnused()`: duyệt các suất, kể cả lịch sử.
- `prepareSeats()`: sinh vị trí bằng chia/lấy dư và kiểm tra đủ miền ID.

Sửa phòng luôn sinh bộ ghế có ID mới, kể cả khi kích thước không đổi; đây là
cách cài đặt, không phải một BR riêng. Chi tiết giữ con trỏ an toàn nằm trong
[thiết kế lớp](class-design.md#6-thay-đổi-phòng-trong-direct-association).

## 2. ScreeningService

Nguồn: [header](../../src/services/ScreeningService.h),
[implementation](../../src/services/ScreeningService.cpp).

| Hàm | Xử lý và quy tắc liên quan |
|---|---|
| `add()` | Tìm phim/phòng, tạo candidate, gọi `validate()`, sau đó `insert()`. BR-S01/S02. |
| `update()` | Gọi `requireNoTickets()`, tìm phim/phòng, kiểm tra candidate mới và gọi `update()`. BR-S01/S02/S03. |
| `erase()` | Gọi `requireNoTickets()` rồi repository xóa. BR-D04. |
| `find()` | Kiểm tra ID, gọi `findById()`, báo lỗi nếu thiếu; không kiểm tra suất còn ở tương lai. |
| `endTime()` | Tra suất, chuyển thời gian sang phút, cộng thời lượng phim rồi chuyển về chuỗi. BR-S04. |
| `list()` | Lọc đồng thời ngày bắt đầu, phim, phòng và `futureOnly`; chỉ là truy vấn. |

`validate()` bỏ qua cùng ID và khác phòng, rồi dùng điều kiện
`start < otherEnd && otherStart < end` để phát hiện giao nhau theo BR-S02.
Sau đó lấy đồng hồ và kiểm tra BR-S01. `requireNoTickets()` tra suất và duyệt vé
để phục vụ cả sửa và xóa.

`validateFilters()` kiểm tra ngày và ID lọc không âm. Ngày rỗng/ID 0 bỏ lọc;
ID dương không tồn tại cho kết quả rỗng. `futureOnly=true` lấy suất bắt đầu
sau giờ hiện tại. Sửa suất kiểm tra giờ mới, không dùng giờ cũ để cấm sửa.

## 3. TicketService

Luồng minh họa: [trình tự bán một vé](../diagrams/ticket-sale-sequence/README.md).
Hình chỉ vẽ nhánh thành công; phần mô tả dưới hình chỉ rõ kiểm tra và lời gọi
được gộp, không coi preview là bước bắt buộc của API sell.

Nguồn: [header](../../src/services/TicketService.h),
[implementation](../../src/services/TicketService.cpp).

| Hàm | Xử lý và quy tắc liên quan |
|---|---|
| `preview()` | Gọi `details()`, lấy giờ hiện tại rồi `requireAvailable()`; trả thông tin xem trước. BR-T01/T02/T03. |
| `sell()` | Kiểm tra cờ nhận tiền, tự kiểm tra lại dữ liệu/khả năng bán, lấy giá và giờ rồi tạo/insert vé. BR-T01–T05. Không bắt buộc gọi `preview()` trước. |
| `find()` | Tra vé rồi gọi `details()` ghép phim/phòng/suất/ghế và giờ kết thúc. Đây là truy vấn tổng hợp, không chỉ chuyển tiếp repository; không kiểm tra khả năng bán. |
| `seatMap()` | Gọi `CatalogService::makeRoomSeatMap()` để dùng lại BR-C02, sau đó đánh dấu `sold` từ vé của suất đang xem; phát hiện liên kết sai hoặc vé trùng. BR-T03. |
| `revenue()` | Lọc ngày từ `soldAt`, đếm vé và cộng `unitPrice`; kiểm tra tràn số. BR-T06. |
| `listBySaleDate()` | Lọc ngày bán rồi gọi `find()` cho từng vé; không thêm điều kiện bán. |

`details()` kiểm tra ID, tìm suất/ghế, kiểm tra BR-T01 và tính giờ kết thúc.
`requireAvailable()` kiểm tra BR-T02/T03. `validateSaleDate()` chỉ gọi helper
kiểm tra ngày. `sell()` chỉ nhận cờ `cashConfirmed`, không xác minh giao dịch
tiền mặt thực tế.

Không có API sửa/xóa vé (BR-D05). Repository vẫn có CRUD chung; chỉ gọi trực
tiếp repository không bảo đảm chính sách của service.

## 4. Kết quả, kiểm tra kỹ thuật và ranh giới

- Danh sách kết quả được tạo tạm rồi `swap` khi truy vấn thành công; service
  không sắp xếp riêng. Các struct `SaleDetails`, `TicketDetails`, `DailyRevenue`
  chỉ chứa kết quả, không phải service.
- [helper/SeatMap](../../src/helper/SeatMap.h) chứa `SeatMap = Matrix2D<SeatStatus>`
  và hàm `makeSeatMap(rows, columns, seats)`. Helper nhận danh sách ghế, không biết
  repository, suất chiếu hay vé; ô chưa điền có `seatId=0`, `sold=false`.
  Nó cấp phát ma trận, chuyển chỉ số từ 1 về 0, từ chối con trỏ rỗng,
  vị trí ngoài ma trận và ghi đè ô đã có ghế. Đây là bảo vệ phép chuyển đổi;
  helper không yêu cầu mọi ô đều có ghế.
  `seatMapCell()` dùng chung phép chuyển tọa độ và truy cập ô; TicketService gọi
  hàm này khi đánh dấu đã bán, không tự tính chỉ số ma trận.
- BR-C02 do CatalogService bảo đảm khi sinh bộ ghế trong `prepareSeats()`;
  `makeRoomSeatMap()` kiểm tra lại số ghế trước khi dựng sơ đồ. Đủ số lượng kết hợp
  với kiểm tra không trùng/không vượt ô của helper bảo đảm đủ mọi vị trí.
  TicketService dùng lại hàm này rồi áp dụng trạng thái đã bán theo suất;
  `requireAvailable()` vẫn là nơi chặn bán trùng, không dựa vào bản sơ đồ cũ.
- Service gọi helper trực tiếp, không thêm thuộc tính, interface hoặc DI cho
  phép chuyển đổi không trạng thái. Phụ thuộc repository vẫn được inject như trước.
- Constructor entity kiểm tra giá trị nội tại; repository kiểm tra ID duy nhất
  và không đổi khi cập nhật. Service không chuyển những việc này thành BR mới.
- Định dạng ngày, tràn số, cấp phát, rollback và từ chối đồng hồ rỗng là an toàn
  kỹ thuật. `DateTimeHelper` không biết entity hoặc quy tắc xếp lịch.
- Các service chỉ thao tác RAM, không đọc/ghi TXT. Phụ thuộc và vòng đời được
  mô tả trong [thiết kế lớp](class-design.md); phạm vi kiểm tra dữ liệu cũ nằm ở
  [thiết kế TXT](txt-format.md).
