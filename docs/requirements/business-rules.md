# Danh mục quy tắc nghiệp vụ

Tài liệu ghi nhận quy tắc **đã có trong code**, không phải đề xuất chức năng mới.
Mã `BR-...` dùng để dẫn chiếu; tên hàm chỉ ra nơi thực hiện quy tắc.
Phạm vi: thao tác qua service; database không kiểm tra lại lịch chiếu khi nạp/lưu.

## 1. Phim, phòng và ghế

Nguồn: [CatalogService.cpp](../../src/services/CatalogService.cpp).

| Mã | Quy tắc hiện tại | Hàm thực hiện |
|---|---|---|
| BR-C01 | Phim còn suất tham chiếu được đổi tên nhưng không được đổi thời lượng, kể cả suất lịch sử. | `updateFilm()`, `filmReferenced()` |
| BR-C02 | Mỗi phòng có đủ ghế theo số hàng × số ghế mỗi hàng; hàng/cột đánh số từ 1. | `addRoom()`, `prepareSeats()` |
| BR-C03 | Phòng còn suất sử dụng không được sửa, kể cả chỉ đổi tên; không phân biệt suất cũ hay mới. | `updateRoom()`, `requireRoomUnused()` |

Khi được sửa phòng, code luôn sinh lại bộ ghế với ID mới, kể cả kích thước không
đổi. Đây là cách cài đặt hiện tại, không phải yêu cầu nghiệp vụ bắt buộc.
Service không cấm trùng tên phim hoặc tên phòng.

## 2. Suất chiếu

Nguồn: [ScreeningService.cpp](../../src/services/ScreeningService.cpp).

| Mã | Quy tắc hiện tại | Hàm thực hiện |
|---|---|---|
| BR-S01 | Khi thêm/sửa, phim và phòng phải tồn tại; giờ bắt đầu mới phải sau giờ hiện tại. | `add()`, `update()`, `validate()` |
| BR-S02 | Cùng phòng không được có hai khoảng sử dụng giao nhau. Khoảng sử dụng là `[bắt đầu, bắt đầu + thời lượng phim + 15 phút nghỉ)`. Tiếp giáp đúng ranh giới được phép; khác phòng được trùng giờ. | `validate()` |
| BR-S03 | Suất đã có vé không được sửa, kể cả đã qua giờ chiếu. | `update()`, `requireNoTickets()` |
| BR-S04 | Giờ kết thúc chiếu bằng giờ bắt đầu + thời lượng phim, không gồm 15 phút nghỉ. | `endTime()` |

Khi sửa, không so trùng lịch với chính suất đó. Suất lịch sử chưa có vé vẫn có
thể chuyển sang tương lai: code kiểm tra giờ mới, không cấm riêng thao tác này.

## 3. Vé và doanh thu

Nguồn: [TicketService.cpp](../../src/services/TicketService.cpp).

| Mã | Quy tắc hiện tại | Hàm thực hiện |
|---|---|---|
| BR-T01 | Ghế phải thuộc phòng của suất và nằm trong kích thước phòng. | `details()` |
| BR-T02 | Chỉ bán trước giờ bắt đầu; tại đúng giờ bắt đầu cũng bị từ chối. | `requireAvailable()` |
| BR-T03 | Mỗi ghế chỉ bán một lần trong một suất; vé ở suất khác không làm ghế đó hết chỗ. | `requireAvailable()`, `seatMap()` |
| BR-T04 | Phải xác nhận đã nhận tiền mặt mới được bán. Code chỉ kiểm tra cờ `cashConfirmed`, không kiểm chứng tiền thực tế. | `sell()` |
| BR-T05 | Vé chốt giá hiện tại của suất và thời điểm bán từ đồng hồ khi bán. | `sell()` |
| BR-T06 | Doanh thu ngày là tổng giá lưu trên các vé bán trong ngày đó, không phải ngày chiếu hoặc giá hiện tại của suất. Không có vé thì số vé và doanh thu bằng 0. | `revenue()` |

`preview()` kiểm tra BR-T01–BR-T03 nhưng không tạo vé hay giữ ghế. `sell()` tự
kiểm tra lại, không bắt buộc gọi `preview()` trước. Khi tra vé, tên phim/phòng
lấy từ dữ liệu hiện tại; chỉ giá và thời điểm bán được chốt trên vé.

## 4. Quy ước xóa dữ liệu

| Mã | Đối tượng | Quy tắc hiện tại | Nơi thực hiện |
|---|---|---|---|
| BR-D01 | Phim | Chỉ xóa nếu không còn suất tham chiếu; không xóa kèm suất hoặc vé. | `CatalogService::eraseFilm()` |
| BR-D02 | Phòng | Chỉ xóa nếu không còn suất sử dụng; xóa kèm toàn bộ ghế thuộc phòng. | `CatalogService::eraseRoom()` |
| BR-D03 | Ghế | Không có thao tác xóa riêng; quản lý theo kích thước và vòng đời phòng. | API của `CatalogService` |
| BR-D04 | Suất chiếu | Chỉ xóa nếu chưa có vé, kể cả suất đã qua giờ chiếu. | `ScreeningService::erase()` |
| BR-D05 | Vé | Không cung cấp sửa/xóa; giữ để tra cứu và tính doanh thu. | API của `TicketService` |

Quy tắc áp dụng cả dữ liệu lịch sử. Muốn xóa phim/phòng, phải xóa các suất chưa
có vé trước; nếu còn suất có vé thì giữ dữ liệu liên quan. Không chuyển liên kết
thành `NULL`, không dùng bản ghi mặc định và không có xóa mềm.
Xóa thành công tác động RAM; chỉ ghi TXT khi lưu.

## 5. Những hàm không có quy tắc nghiệp vụ riêng

Không gán mã BR cho các thao tác chỉ điều phối CRUD, tra cứu hoặc xử lý kỹ thuật.

| Service | Hàm | Vai trò thực tế |
|---|---|---|
| `CatalogService` | `addFilm()` | Lấy ID, tạo `Film`, gọi `insert()`; không có điều kiện nghiệp vụ bổ sung ở service. |
| `CatalogService` | `film()`, `room()` | Kiểm tra ID, gọi `findById()`, báo lỗi nếu không tìm thấy. |
| `CatalogService` | `films()`, `rooms()` | Lấy danh sách; `films()` lọc tên chứa chuỗi, phân biệt hoa/thường, chuỗi rỗng lấy tất cả. |
| `CatalogService` | `seatMap()` | Tra phòng và gọi `makeSeatMap()`; không xác định ghế đã bán theo suất. |
| `ScreeningService` | `find()` | Tra cứu qua repository, không kiểm tra suất còn ở tương lai. |
| `ScreeningService` | `list()`, `validateFilters()` | Lọc đồng thời ngày bắt đầu, phim, phòng, `futureOnly`. Ngày rỗng/ID 0 bỏ lọc; ID âm hoặc ngày sai bị từ chối; ID dương không tồn tại trả rỗng. `futureOnly` lấy suất bắt đầu sau hiện tại. |
| `TicketService` | `find()` | Tra vé rồi ghép chi tiết qua `details()`, không chỉ chuyển tiếp repository. Không kiểm tra khả năng bán nên tra được vé lịch sử. |
| `TicketService` | `listBySaleDate()` | Lọc ngày bán và ghép chi tiết từng vé; không thêm điều kiện bán. |
| `TicketService` | `validateSaleDate()` | Chỉ gọi helper kiểm tra ngày. |

`makeSeatMap()` là hàm dựng ma trận, không phải service; kiểm tra đủ ghế, không
trùng/vượt vị trí. Các struct kết quả cũng không phải service.
Kiểm tra giá trị nội tại do constructor entity thực hiện; ID duy nhất và không
đổi khi sửa do repository đảm bảo. Kiểm tra định dạng ngày, tràn số, đồng hồ
rỗng và rollback là an toàn kỹ thuật, không phải quy tắc kinh doanh riêng.
Các truy vấn service hiện tại không sắp xếp kết quả riêng.
