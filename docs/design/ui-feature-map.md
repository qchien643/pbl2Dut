# Feature map theo tab giao diện

Đối chiếu tìm tên phim ngày **28/11/2026**, các phần còn lại **27/11/2026**.
Tài liệu ghi **hành vi đã có**, không phải
danh sách tính năng dự định làm. Các mã `UI-*` chỉ dùng để chỉ đúng dòng khi
thảo luận, không thay thế mã yêu cầu FR hoặc quy tắc BR.

SRS giữ phạm vi yêu cầu; [Business Rules](../requirements/business-rules.md) giữ
chính sách; [hướng dẫn sử dụng](../guides/user-guide.md) giữ trình tự thao tác
và ảnh. Tài liệu này mô tả đầu vào, cách khớp/lọc, kết quả và giới hạn từng tab.

## Cách đọc và thảo luận

Thứ tự đọc đề xuất: [Phim](#1-phim) → [Phòng & ghế](#2-phòng--ghế) →
[Suất chiếu](#3-suất-chiếu) → [Bán vé](#4-bán-vé) →
[Vé và doanh thu](#5-vé-và-doanh-thu).
Trên ứng dụng, tab Bán vé vẫn đứng đầu; tài liệu không đổi thứ tự giao diện.

Mỗi tab có bảng hiện trạng và câu hỏi chưa chốt. Khi trao đổi, nêu mã `UI-*`,
điều muốn giữ/đổi và ví dụ kết quả mong muốn. Chỉ sau khi thống nhất mới cập nhật
code và tài liệu; câu hỏi dưới đây không tự trở thành yêu cầu mới.

### Quy ước chung đã có

- Mã bản ghi được sinh tự động; ô tra mã nhận số nguyên dương trong miền int64.
- Danh sách chọn phim/phòng hiển thị `#mã — tên`, nhưng chọn đúng một đối tượng
  theo ID; không phải ô tìm tên tự do. “Tất cả” nghĩa là bỏ điều kiện tương ứng.
- Các bộ lọc đang bật kết hợp theo **AND**: bản ghi phải thỏa tất cả điều kiện.
- Bảng danh sách chỉ đọc; chọn một dòng để thao tác, không sửa trực tiếp ô.
- Thêm/sửa mở hộp nhập riêng; khi lỗi, hiển thị thông báo để sửa hoặc hủy.
  Xóa cần xác nhận. Mọi thay đổi chỉ nằm trên RAM đến khi lưu thành công.

## 1. Phim

Liên quan FR-01. Khi mở: chọn chế độ **Tên phim**, ô tìm trống, hiển thị toàn bộ.
Bảng gồm **Mã, Tên phim, Thời lượng (phút)**.

| Mã | Chức năng | Đầu vào / thao tác | Hành vi và kết quả hiện tại |
|---|---|---|---|
| UI-F01 | Tìm theo tên | Chọn “Tên phim”, nhập các từ có ít nhất 3 ký tự | Không phân biệt dấu/hoa thường; các từ kết hợp AND, không phụ thuộc thứ tự. Khớp tiền tố/chuỗi con hoặc lỗi một thao tác trên nguyên từ còn trigram chung. Đổi nội dung cập nhật ngay; từ ngắn báo hướng dẫn. |
| UI-F02 | Tìm theo mã | Chọn “Mã phim”, nhập ID | Khớp chính xác một ID, không tìm các mã chứa đoạn số; tối đa một phim. Mã sai định dạng báo lỗi, mã không tồn tại cho danh sách rỗng. |
| UI-F03 | Xem tất cả | Xóa nội dung ô tìm | Hiển thị toàn bộ phim, kể cả đang chọn chế độ tìm theo mã. |
| UI-F04 | Thêm phim | Nhập tên và thời lượng nguyên dương | Tạo phim với mã tự sinh; thành công thì xóa từ tìm, chọn phim mới trong bảng. BR-C04. |
| UI-F05 | Sửa phim | Chọn dòng rồi “Sửa phim”, hoặc nhấp đúp dòng | Hộp nhập có tên/thời lượng hiện tại; không cho đổi ID. Thành công thì xóa từ tìm và chọn lại phim. BR-C01/C04. |
| UI-F06 | Xóa phim | Chọn dòng, bấm “Xóa phim”, xác nhận | Xóa đúng phim được chọn nếu service cho phép; không xóa kèm suất/vé. BR-D01. |

**Ví dụ:** giả sử có `Avatar` và `Avatar 2`, nhập `Avatar` ở chế độ tên trả cả hai;
nhập `avatar` cũng khớp hai tên đó. Nhập `12` ở chế độ mã chỉ tìm ID 12,
không tìm ID 120 hoặc phim có chữ “12” trong tên.

**Giới hạn:** chưa lọc khoảng thời lượng; không bảo đảm mọi lỗi gõ hoặc lỗi mất
hết trigram chung. Từ tìm dưới ba ký tự không được xử lý bằng index ngắn hơn.
Nút sửa/xóa được bật khi chọn dòng; bật nút không có nghĩa thao tác chắc chắn
được nghiệp vụ cho phép, service vẫn kiểm tra khi thực hiện.

**Đối chiếu:** [CinemaWindow.cpp](../../src/gui/CinemaWindow.cpp) —
`setupFilmTab`, `refreshFilms`, `editFilm`, `deleteFilm`;
[CatalogService.cpp](../../src/services/CatalogService.cpp) — `films`, `film`.

**Đã triển khai thử (28/11/2026):** xem
[thiết kế index tên phim](film-name-search.md) và
[yêu cầu FR-01 cập nhật](../requirements/srs.md) theo năm ví dụ `nguyen trai`,
`trai nguyen`, `nguy`, `guy`, `nguyn trai`. Dùng index trigram, không thêm khóa
1–2 ký tự hoặc nguyên từ. [Benchmark](../testing/film-search-benchmark.md) có cả
trường hợp index chậm hơn scan. Hai chế độ tên/mã đã đủ dễ hiểu chưa?

## 2. Phòng & ghế

Liên quan FR-02. Hiển thị toàn bộ phòng; **không có ô tìm tên/mã phòng**.
Bảng gồm **Mã, Tên phòng, Số hàng, Ghế / hàng, Sức chứa**.

| Mã | Chức năng | Đầu vào / thao tác | Hành vi và kết quả hiện tại |
|---|---|---|---|
| UI-R01 | Xem danh sách phòng | Vào tab | Hiển thị tất cả phòng, không lọc theo tên, ID hay sức chứa. |
| UI-R02 | Xem sơ đồ ghế | Chọn một phòng | Hiển thị ma trận hàng/cột của phòng; chú giải khi rê chuột lên ghế có mã ghế. Không xác định đã bán vì chưa chọn suất. |
| UI-R03 | Thêm phòng | Nhập tên, số hàng, ghế mỗi hàng | Tạo phòng và tự sinh toàn bộ ghế hình chữ nhật; ID tự sinh. BR-C02/C05. |
| UI-R04 | Sửa phòng | Chọn dòng rồi sửa, hoặc nhấp đúp | Sửa tên/kích thước qua hộp nhập; sinh lại bộ ghế khi được phép. BR-C02/C03/C05. |
| UI-R05 | Xóa phòng | Chọn dòng, bấm xóa, xác nhận | Xóa phòng cùng ghế của phòng nếu được phép; không xóa kèm suất. BR-D02/D03. |

**Chưa có:** thêm/sửa/xóa từng ghế, chọn ghế để bán ngay tại tab này,
sơ đồ lối đi hoặc bố trí ghế khuyết. Bấm ô ghế không phát hành vé.

**Đối chiếu:** [CinemaWindow.cpp](../../src/gui/CinemaWindow.cpp) —
`setupRoomTab`, `refreshRooms`, `refreshRoomSeats`, `editRoom`, `deleteRoom`;
[CatalogService.cpp](../../src/services/CatalogService.cpp) — `rooms`, `seatMap`.

**Cần thảo luận, chưa chốt:** danh sách phòng có cần ô tìm kiếm không?
Người dùng có phân biệt rõ sơ đồ bố trí ở đây với sơ đồ bán vé không?

## 3. Suất chiếu

Liên quan FR-03. Mặc định chưa bật lọc ngày, mã để trống, phim/phòng là “Tất cả”;
hiển thị cả suất cũ và tương lai. Bảng gồm **Mã, Phim, Phòng, Bắt đầu,
Kết thúc, Giá vé (VND)**.

| Mã | Chức năng | Đầu vào / thao tác | Hành vi và kết quả hiện tại |
|---|---|---|---|
| UI-S01 | Tìm theo mã suất | Nhập mã chính xác | Chỉ giữ suất đúng ID, đồng thời vẫn phải thỏa ngày/phim/phòng đang lọc. Đổi nội dung là cập nhật ngay. |
| UI-S02 | Lọc ngày chiếu | Bật “Ngày chiếu”, chọn một ngày | So ngày **bắt đầu** của suất. Bỏ chọn thì không lọc ngày; không chọn một khoảng nhiều ngày. |
| UI-S03 | Lọc phim | Chọn một phim hoặc “Tất cả phim” | Chỉ giữ suất tham chiếu đúng phim đã chọn; không tìm chuỗi tên phim trong tab này. |
| UI-S04 | Lọc phòng | Chọn một phòng hoặc “Tất cả phòng” | Chỉ giữ suất thuộc phòng đã chọn; có thể kết hợp với ba điều kiện trên. |
| UI-S05 | Thêm suất | Chọn phim, phòng; nhập ngày giờ bắt đầu, giá nguyên dương | Tạo suất, tự sinh mã; service kiểm tra giờ và xung đột lịch. BR-S01/S02. |
| UI-S06 | Sửa suất | Chọn dòng rồi sửa, hoặc nhấp đúp | Hộp nhập cho đổi phim/phòng/giờ/giá; không đổi ID. BR-S01/S02/S03. |
| UI-S07 | Xóa suất | Chọn dòng, bấm xóa, xác nhận | Xóa đúng suất được chọn nếu service cho phép. BR-D04. |

**Ví dụ:** suất 12 chiếu ngày 01/12 ở phòng 2. Nhập mã 12 nhưng lọc phòng 1
thì kết quả rỗng; tìm mã không tự tắt điều kiện phòng. Suất bắt đầu 23:30 ngày
01/12 và kết thúc ngày 02/12 vẫn thuộc bộ lọc ngày 01/12.

**Chưa có:** lọc khoảng giờ/ngày, khoảng giá hoặc nút chỉ xem suất tương lai
trong tab quản lý này. Điều kiện chỉ suất chưa bắt đầu được dùng ở tab Bán vé.

**Đối chiếu:** [CinemaWindow.cpp](../../src/gui/CinemaWindow.cpp) —
`setupScreeningTab`, `refreshScreenings`, `editScreening`, `deleteScreening`;
[ScreeningService.cpp](../../src/services/ScreeningService.cpp) — `list`.

**Cần thảo luận, chưa chốt:** nhập mã nên tiếp tục kết hợp các bộ lọc, hay nên
đi thẳng tới suất như cách tìm mã vé? Có cần cách xóa toàn bộ bộ lọc dễ thấy hơn?

## 4. Bán vé

Liên quan FR-04. Mặc định lọc **ngày chiếu hôm nay**, tất cả phim/phòng.
Chỉ lấy suất có giờ bắt đầu **sau giờ hiện tại**; bỏ lọc ngày cũng không hiển thị
suất đã bắt đầu. Bảng suất có các cột giống tab Suất chiếu.

| Mã | Chức năng | Đầu vào / thao tác | Hành vi và kết quả hiện tại |
|---|---|---|---|
| UI-B01 | Lọc suất để bán | Ngày chiếu tùy chọn, phim, phòng | Kết hợp các bộ lọc và điều kiện chưa bắt đầu. Không có ô nhập mã suất ở tab này. |
| UI-B02 | Xem ghế của suất | Chọn một suất trong bảng | Hiển thị `○` trống, `×` đã bán theo **suất đang chọn**, không phải trạng thái chung của phòng. |
| UI-B03 | Xem trước vé | Bấm một ghế trống | Chọn một ghế, đổi ký hiệu thành `✓`, hiển thị phim/phòng/giờ/ghế/giá. Chưa tạo vé hoặc giữ chỗ. |
| UI-B04 | Xác nhận bán | Bấm “Xác nhận đã nhận tiền mặt”, xác nhận hộp thoại | Service kiểm tra lại điều kiện, tạo **một vé** cho ghế đã chọn và trả mã vé. UI cập nhật ghế đã bán, đồng thời cập nhật phần tra vé. BR-T01–T05. |
| UI-B05 | Đổi suất hoặc bộ lọc | Chọn lại suất/ngày/phim/phòng | Làm mới danh sách/sơ đồ tương ứng, bỏ lựa chọn ghế cũ; phải chọn ghế lại trước khi bán. |

**Ví dụ:** cùng một ghế có thể là `×` ở suất A nhưng `○` ở suất B. Không có suất
hôm nay không đồng nghĩa không có lịch: có thể chọn ngày khác hoặc bỏ lọc ngày.

**Chưa có:** chọn nhiều ghế cùng lúc, giữ chỗ, mua theo nhóm, thanh toán điện tử,
in vé hoặc hủy giao dịch đã phát hành. Nút xác nhận tiền mặt không kết nối hệ
thống thanh toán. Vé mới bán vẫn chưa được ghi TXT cho đến khi lưu.

**Đối chiếu:** [CinemaWindow.cpp](../../src/gui/CinemaWindow.cpp) —
`setupSaleTab`, `refreshSaleScreenings`, `refreshSaleSeats`, `selectSaleSeat`,
`sellTicket`; [TicketService.cpp](../../src/services/TicketService.cpp) —
`preview`, `seatMap`, `sell`.

**Cần thảo luận, chưa chốt:** mặc định lọc hôm nay có làm người dùng tưởng thiếu
dữ liệu không? Luồng chọn một ghế và xác nhận tiền đã rõ ràng chưa?

## 5. Vé và doanh thu

Liên quan FR-05. Mặc định chọn **ngày bán hôm nay**. Bảng gồm **Mã vé, Giờ bán,
Phim, Phòng, Ghế, Giá đã bán**; khu vực bên cạnh hiển thị chi tiết vé.

| Mã | Chức năng | Đầu vào / thao tác | Hành vi và kết quả hiện tại |
|---|---|---|---|
| UI-V01 | Xem tất cả vé bán trong ngày | Chọn một “Ngày bán” | Liệt kê vé có thời điểm bán thuộc ngày đó, bất kể ngày chiếu. Không có vé thì bảng rỗng. |
| UI-V02 | Xem tổng ngày | Cùng ngày đang chọn | Đếm vé, cộng giá chốt trên vé; không lấy giá suất hiện tại. Không có vé thì số vé và doanh thu bằng 0. BR-T06. |
| UI-V03 | Đi đến mã vé | Nhập ID chính xác rồi bấm “Tìm vé” hoặc Enter | Tra vé không bị giới hạn bởi ngày đang chọn; nếu thấy thì tự chuyển sang ngày bán của vé, chọn dòng và mở chi tiết. Mã sai/không tồn tại báo lỗi. |
| UI-V04 | Xem chi tiết | Chọn một dòng vé | Hiển thị mã vé, phim, mã suất, giờ bắt đầu/kết thúc, phòng, ghế, thời điểm bán và giá đã bán; điền mã vào ô tra cứu. |
| UI-V05 | Đổi ngày báo cáo | Chọn ngày khác | Cập nhật danh sách và tổng, xóa mã đang nhập và nội dung chi tiết cũ; không tạo danh sách tra cứu thứ hai. |

**Ví dụ:** vé bán ngày 27/11 cho suất ngày 01/12 xuất hiện ở báo cáo 27/11.
Nếu đang xem 28/11 mà tìm đúng mã vé đó, giao diện chuyển về 27/11.

**Chưa có:** lọc theo khoảng nhiều ngày, phim/phòng/ghế hoặc ngày chiếu;
thống kê nhóm theo phim/phòng; xuất báo cáo. Không sửa, xóa, hủy hoặc hoàn vé
đã bán trên UI (BR-D05).

**Đối chiếu:** [CinemaWindow.cpp](../../src/gui/CinemaWindow.cpp) —
`setupReportTab`, `lookupTicket`, `showTicketDetails`, `refreshRevenue`;
[TicketService.cpp](../../src/services/TicketService.cpp) — `find`,
`listBySaleDate`, `revenue`.

**Cần thảo luận, chưa chốt:** nhãn “Đi đến mã vé” và việc tự đổi ngày đã dễ hiểu
chưa? Phạm vi báo cáo một ngày có đủ cho đồ án không?

## 6. Chức năng dùng chung và lưu dữ liệu

| Mã | Chức năng | Đầu vào / thao tác | Hành vi hiện tại |
|---|---|---|---|
| UI-G01 | Mở dữ liệu | Chạy ứng dụng, tùy chọn thư mục dữ liệu | Mặc định `data/demo`; nạp trước khi mở cửa sổ. Thư mục không có bộ file cần xác nhận phiên rỗng; lỗi nạp không cho mở chỉnh sửa. FR-06. |
| UI-G02 | Xem trạng thái lưu | Thanh trạng thái / tiêu đề cửa sổ | Báo có thay đổi chưa lưu; không tự lưu sau mỗi thao tác. |
| UI-G03 | Lưu và thoát | Bấm nút hoặc đóng cửa sổ rồi xác nhận | Ghi toàn bộ dữ liệu. Hủy xác nhận thì ở lại; lỗi lưu thì giữ cửa sổ và RAM để thử lại. FR-07. |

Chi tiết phục hồi thuộc [thiết kế TXT](txt-format.md); không diễn giải lại cơ chế
lưu file trong feature map này.

## 7. Điểm kỹ thuật cần lưu ý khi thảo luận truy vấn

Đây là cách thực hiện hiện tại, không phải chức năng hay index mới được đề xuất:

| Chức năng | Điều kiện | Cách thực hiện |
|---|---|---|
| UI-F02 | ID phim bằng giá trị nhập | Tra hash index ID qua `CatalogService::film()` |
| UI-F01 | Từ/chuỗi con hoặc một lỗi gõ trong phạm vi FR-01 | Tra trigram → Film*, hợp/giao theo ID rồi xác nhận trên normalizedTitle_ có sẵn; không quét/chuẩn hóa lại mọi tên |
| UI-S01 | ID suất bằng giá trị nhập và các bộ lọc khác | **Vẫn lấy danh sách qua service rồi UI lọc ID**, không tra index ID trực tiếp để chọn suất |
| UI-S02–S04, UI-B01 | Ngày, phim, phòng; có thể thêm điều kiện tương lai | Duyệt danh sách suất và lọc |
| UI-R02, UI-B02 | Ghế thuộc phòng; vé thuộc suất | Duyệt danh sách ghế/vé; helper chỉ dựng ma trận |
| UI-V03 | ID vé bằng giá trị nhập | Tra hash index ID để tìm vé, sau đó vẫn lọc ngày để cập nhật cả bảng và tổng |
| UI-V01–V02 | Ngày bán bằng ngày chọn | Duyệt vé để lập danh sách và tính tổng |

Tra đúng một ID có thể dùng index; điều đó không có nghĩa toàn bộ lần cập nhật
màn hình là O(1). Phạm vi DI và index xem [thiết kế lớp](class-design.md).

## 8. Theo dõi buổi thảo luận

| Tab | Trạng thái | Kết luận thay đổi |
|---|---|---|
| Phim | Đã triển khai thử [index tên](film-name-search.md) | Đủ năm ví dụ; có benchmark và giới hạn từ ngắn/lỗi gõ |
| Phòng & ghế | Chưa thảo luận | Chưa chốt |
| Suất chiếu | Chưa thảo luận | Chưa chốt |
| Bán vé | Chưa thảo luận | Chưa chốt |
| Vé và doanh thu | Chưa thảo luận | Chưa chốt |

Bảng hiện trạng được đối chiếu từ code; lượt cập nhật tài liệu này không chạy
thao tác Qt bằng tay và không thay đổi chức năng giao diện.
