# SOFTWARE REQUIREMENTS SPECIFICATION (SRS)

## ỨNG DỤNG QUẢN LÝ RẠP CHIẾU PHIM

| Thông tin | Nội dung |
|---|---|
| Mã tài liệu | SRS-CMS-001 |
| Phiên bản | 1.0 |
| Ngày cập nhật tài liệu | 26/10/2026 |
| Công nghệ | C++17, Qt 6 Widgets, CMake |
| Triển khai | Offline, một máy, một phiên chạy |
| Lưu trữ | TXT UTF-8; nạp khi mở, lưu khi thoát |

## 1. Mục tiêu và phạm vi

Ứng dụng phục vụ một người vận hành rạp nhỏ, đồng thời minh họa OOP, thiết kế dữ
liệu và cấu trúc dữ liệu tự cài đặt.

Phần database của bài thể hiện qua mô hình quan hệ, khóa, toàn vẹn và truy vấn
bằng code; không có DBMS thực thi SQL. Lập luận OOP và chuẩn hóa nằm trong tài
liệu thiết kế, không phải một chức năng bổ sung của sản phẩm.

Chức năng gồm:

1. Quản lý phim.
2. Quản lý phòng và sơ đồ ghế hình chữ nhật.
3. Quản lý lịch chiếu.
4. Bán từng vé theo suất và ghế.
5. Xem vé đã bán, tra cứu vé và doanh thu theo ngày bán.
6. Nạp dữ liệu khi mở và lưu toàn bộ khi thoát.

Ngoài phạm vi: đăng nhập/phân quyền, đặt vé online, nhiều chi nhánh, nhiều phiên
đồng thời, giỏ hàng, giữ ghế, hủy/hoàn vé, ghế VIP, khuyến mãi, đồ ăn, DBMS,
thanh toán điện tử, in vé và báo cáo nâng cao.

## 2. Mô hình vận hành

- Actor duy nhất: **Người vận hành**.
- Dữ liệu được nạp từ TXT vào RAM khi mở; mọi thao tác trong phiên chỉ thay đổi RAM.
- Nút `Lưu và thoát` hoặc đóng cửa sổ sẽ lưu đủ bộ TXT; chỉ đóng khi lưu thành công.
- Mất điện hoặc buộc dừng trước khi lưu có thể làm mất thay đổi trong phiên.
- Không mở hai phiên hoặc sửa TXT bên ngoài khi chương trình đang chạy.
- Thời gian dùng giờ địa phương, định dạng `YYYY-MM-DD HH:mm`; tiền là số nguyên VND.

## 3. Yêu cầu chức năng

### 3.1. Phim

| ID | Yêu cầu |
|---|---|
| FR-FILM-01 | Thêm phim với mã duy nhất, tên và thời lượng phút. |
| FR-FILM-02 | Từ chối mã trùng, tên rỗng hoặc thời lượng không dương. |
| FR-FILM-03 | Liệt kê và tìm theo mã hoặc một phần tên. |
| FR-FILM-04 | Cho sửa tên; chỉ sửa thời lượng khi phim chưa có suất. |
| FR-FILM-07 | Chỉ xóa phim chưa có suất. |

### 3.2. Phòng và ghế

| ID | Yêu cầu |
|---|---|
| FR-ROOM-01 | Tạo phòng với mã, tên, số hàng và số ghế mỗi hàng; tự sinh sơ đồ ghế. |
| FR-ROOM-02 | Từ chối mã trùng, tên rỗng, kích thước không dương, tràn số hoặc vị trí ghế trùng. |
| FR-ROOM-03 | Sức chứa bằng số hàng nhân số ghế mỗi hàng. |
| FR-ROOM-04 | Liệt kê phòng và hiển thị sơ đồ ghế theo hàng, cột. |
| FR-ROOM-07 | Chỉ sửa phòng chưa có suất; tạo lại ghế nhất quán trên RAM. |
| FR-ROOM-08 | Chỉ xóa phòng chưa có suất; xóa kèm ghế của phòng. |

### 3.3. Suất chiếu

| ID | Yêu cầu |
|---|---|
| FR-SHOW-01 | Tạo suất với mã, phim, phòng, giờ bắt đầu và giá vé dương. |
| FR-SHOW-02 | Tính giờ kết thúc từ giờ bắt đầu và thời lượng phim. |
| FR-SHOW-03 | Phòng được giữ đến giờ kết thúc cộng 15 phút. |
| FR-SHOW-04 | Từ chối lịch chồng lấn trong cùng phòng, kể cả qua ngày mới. |
| FR-SHOW-05 | Từ chối mã trùng, tham chiếu sai, thời gian sai hoặc giờ bắt đầu không ở tương lai khi tạo/sửa. |
| FR-SHOW-07 | Liệt kê, tìm theo mã và lọc theo ngày, phim, phòng. |
| FR-SHOW-08 | Không sửa/xóa suất đã có vé; suất chưa có vé vẫn phải được kiểm tra lại khi sửa. |

### 3.4. Bán vé

| ID | Yêu cầu |
|---|---|
| FR-SEARCH-01 | Lọc và chọn suất chưa bắt đầu theo ngày, phim hoặc phòng. |
| FR-SEARCH-02 | Hiển thị phim, phòng, thời gian và giá của suất. |
| FR-SEARCH-03 | Hiển thị ghế trống, đã bán và đang chọn. |
| FR-SALE-01 | Chỉ bán trước giờ bắt đầu suất. |
| FR-SALE-02 | Mỗi lần xác nhận bán đúng một ghế và tạo một vé. |
| FR-SALE-03 | Từ chối ghế sai phòng hoặc đã bán trong cùng suất. |
| FR-SALE-05 | Hiển thị lại phim, phòng, thời gian, ghế và giá trước khi xác nhận. |
| FR-SALE-06 | Hủy xác nhận không tạo vé hoặc thay đổi doanh thu. |
| FR-SALE-07 | Chỉ hoàn tất khi người vận hành xác nhận đã nhận tiền mặt. |
| FR-SALE-08 | Khi xác nhận, kiểm tra lại điều kiện, sinh mã vé, thời điểm bán và chốt giá. |
| FR-SALE-09 | Lỗi bán vé không được để lại vé hoặc trạng thái ghế bán một phần. |

### 3.5. Vé và doanh thu

| ID | Yêu cầu |
|---|---|
| FR-QUERY-02 | Tìm vé theo mã. |
| FR-QUERY-03 | Hiển thị mã vé, phim, suất, phòng, ghế, thời điểm bán và giá đã bán. |
| FR-QUERY-04 | Báo rõ mã không hợp lệ hoặc không tìm thấy. |
| FR-QUERY-06 | Không cho sửa/xóa vé đã bán. |
| FR-REPORT-01 | Chọn ngày bán để xem số vé và doanh thu. |
| FR-REPORT-02 | Liệt kê mọi vé bán trong ngày với mã, giờ bán, phim, phòng, ghế và giá. |
| FR-REPORT-03 | Chọn dòng để xem chi tiết; tìm theo mã phải mở đúng ngày bán và chọn đúng vé. |
| FR-REPORT-04 | Doanh thu là tổng giá đã chốt trên vé theo ngày bán, gồm vé chưa lưu. |
| FR-REPORT-07 | Ngày không có vé trả số lượng và doanh thu bằng 0. |

### 3.6. Nạp và lưu dữ liệu

| ID | Yêu cầu |
|---|---|
| FR-DATA-01 | TXT là lưu trữ bền vững duy nhất; không dùng DBMS hoặc dịch vụ ngoài. |
| FR-DATA-02 | Khi mở, nạp và kiểm tra toàn bộ dữ liệu trước khi cho chỉnh sửa. |
| FR-DATA-04 | Bộ file thiếu, hỏng hoặc không đọc được phải được báo; không bỏ qua hay ghi đè. |
| FR-DATA-07 | Trong phiên chỉ cập nhật RAM, không tự ghi sau từng thao tác. |
| FR-DATA-08 | `Lưu và thoát` và đóng cửa sổ dùng cùng quy trình; chỉ đóng sau khi lưu đủ file. |
| FR-DATA-09 | Lưu lỗi phải giữ RAM và cửa sổ để người dùng xử lý rồi thử lại. |
| FR-DATA-10 | Nếu cả bộ file chưa tồn tại, cho xác nhận phiên rỗng; chỉ tạo file khi lưu. |

## 4. Dữ liệu và quy tắc nghiệp vụ

### 4.1. Thực thể

| Entity | Thuộc tính |
|---|---|
| `Film` | `film_id`, `title`, `duration_minutes` |
| `Auditorium` | `auditorium_id`, `name`, `row_count`, `seats_per_row` |
| `Seat` | `seat_id`, `auditorium_id`, `row_number`, `seat_number` |
| `Screening` | `screening_id`, `film_id`, `auditorium_id`, `start_time`, `ticket_price` |
| `Ticket` | `ticket_id`, `screening_id`, `seat_id`, `sold_at`, `unit_price` |

Sức chứa, giờ kết thúc, trạng thái ghế và doanh thu là dữ liệu tính toán. Không có
entity `User`, `Customer`, `Transaction` hoặc `Report`.

### 4.2. Quy tắc

| ID | Quy tắc |
|---|---|
| RULE-01 | Mã do chương trình sinh, là số dương, duy nhất và không sửa sau khi tạo. |
| RULE-02 | Vị trí `(phòng, hàng, cột)` là duy nhất. |
| RULE-03 | Suất phải tham chiếu phim và phòng tồn tại. |
| RULE-04 | Khoảng dùng phòng `[bắt đầu, kết thúc + 15 phút)` không được giao nhau. |
| RULE-05 | Không bán khi giờ hiện tại bằng hoặc sau giờ bắt đầu. |
| RULE-06 | Cặp `(screening_id, seat_id)` là duy nhất; ghế phải thuộc phòng của suất. |
| RULE-09 | Vé chỉ tồn tại sau khi xác nhận đã nhận tiền. |
| RULE-10 | Hủy hoặc lỗi bán không làm thay đổi vé và doanh thu. |
| RULE-12 | Doanh thu dùng `unit_price` trên vé, không dùng lại giá hiện tại của suất. |
| RULE-16 | Không xóa dữ liệu đang được tham chiếu; ghế chỉ xóa cùng phòng chưa có suất. |

Suất lịch sử vẫn được nạp để tra cứu; điều kiện “ở tương lai” chỉ áp dụng khi tạo
hoặc sửa suất.

## 5. Ràng buộc triển khai

### 5.1. Kiến trúc

| Phần | Trách nhiệm |
|---|---|
| `entities/` | Mỗi entity một cặp `.h/.cpp`, giữ và kiểm tra giá trị của chính nó. |
| `DataStructures/` | DSA template tự viết, độc lập nghiệp vụ và Qt. |
| `repositories/` | Mỗi repository một header riêng; template dùng chung, TicketRepository có `.cpp`; giữ bản ghi và chỉ mục trên RAM. |
| `persistence/` | Đọc, ghi và kiểm tra định dạng TXT. |
| `services/` | Quy tắc danh mục, lịch chiếu, bán vé và doanh thu. |
| `ApplicationData` | Sở hữu repository, điều phối nạp và lưu toàn bộ. |
| `ui/` | Giao diện Qt gọi service; không trực tiếp xử lý nghiệp vụ hoặc TXT. |

### 5.2. Cấu trúc dữ liệu

| ID | Yêu cầu |
|---|---|
| DSA-01 | Tự cài đặt cấu trúc chính và quản lý bộ nhớ an toàn. |
| DSA-02 | Cấu trúc dùng template và tái sử dụng cho nhiều kiểu entity. |
| DSA-03 | Dùng `LinkedList<T>`, `HashTable<Key, Value>` và `Matrix2D<T>`. |
| DSA-04 | Xử lý va chạm, dựng lại và cập nhật chỉ mục nhất quán. |
| DSA-05 | Không thêm queue, stack, tree hoặc graph khi nghiệp vụ không cần. |

Không dùng container STL/Qt làm nơi lưu entity hoặc chỉ mục chính.

### 5.3. TXT

Bộ dữ liệu gồm `films.txt`, `auditoriums.txt`, `seats.txt`, `screenings.txt`
và `tickets.txt`. File dùng UTF-8, dấu phân cách `|`; chi tiết định dạng và phục
hồi nằm trong `docs/design/txt-format.md`.

## 6. Giao diện và chất lượng

Giao diện gồm năm tab: `Bán vé`, `Phim`, `Phòng & ghế`, `Suất chiếu`,
`Vé và doanh thu`.

- Trường bắt buộc và lỗi nhập phải rõ ràng.
- Trạng thái ghế dùng cả ký hiệu và màu.
- Hiển thị thông tin vé trước khi xác nhận bán.
- Hiển thị trạng thái chưa lưu.
- Tab `Vé và doanh thu` dùng chung ngày bán cho tổng quan, danh sách và chi tiết.

| ID | Yêu cầu |
|---|---|
| NFR-01 | Hoạt động hoàn toàn offline. |
| NFR-02 | Với 100 phim, 10 phòng × 200 ghế, 1.000 suất và 10.000 vé, mỗi lần tra cứu hoặc thống kê hoàn tất trong 2 giây trên máy demo. |
| NFR-03 | Sau mỗi thao tác, khóa, tham chiếu và chỉ mục trên RAM phải nhất quán. |
| NFR-04 | Sau khi lưu và mở lại, dữ liệu và thống kê không thay đổi. |
| NFR-07 | Lỗi nhập, file và lưu phải được báo; không âm thầm mất dữ liệu. |
| NFR-11 | Không rò rỉ/dùng sai bộ nhớ; kiểm tra tràn khi tính kích thước, ID và doanh thu. |

## 7. Nghiệm thu

| ID | Kịch bản | Kết quả |
|---|---|---|
| SAT-01 | Thêm phim, phòng 3 × 4; thử dữ liệu sai. | Có 12 ghế; dữ liệu sai bị từ chối. |
| SAT-02 | Tạo suất tương lai hợp lệ. | Tính đúng kết thúc, giá và hiển thị ngay. |
| SAT-03 | Tạo lịch chồng lấn, sát ranh giới và qua ngày. | Chồng lấn bị từ chối; đúng ranh giới được nhận. |
| SAT-04 | Bán ghế trống và xác nhận tiền. | Tạo một vé; ghế và doanh thu cập nhật trên RAM. |
| SAT-05 | Bán lại cùng ghế trong cùng suất. | Không tạo vé hoặc tăng doanh thu. |
| SAT-06 | Bán cùng ghế ở suất khác. | Được phép. |
| SAT-07 | Hủy trước xác nhận. | Không tạo vé hoặc giữ ghế. |
| SAT-08 | Thay đổi dữ liệu, lưu, mở lại. | Giữ đúng dữ liệu và thống kê. |
| SAT-10 | Xem ngày có/không có vé; chọn dòng và tìm mã. | Tổng đúng ngày bán; tra cứu mở đúng vé. |
| SAT-11 | Xác nhận bán khi suất đã bắt đầu. | Không tạo vé. |
| SAT-13 | Sửa/xóa dữ liệu đang được tham chiếu. | Bị từ chối, không thay đổi một phần. |
| SAT-14 | Nạp TXT sai hoặc thiếu file. | Báo lỗi và không mở chế độ chỉnh sửa. |
| SAT-15 | Đóng cửa sổ khi lưu/thay file lỗi. | Giữ cửa sổ, RAM và dữ liệu phục hồi. |
| SAT-16 | Mở thư mục chưa có bộ file. | Cho xác nhận phiên rỗng; tạo file khi lưu. |

Chi tiết thiết kế và đối chiếu kiểm thử:

- `docs/design/data-design.md`
- `docs/design/class-design.md`
- `docs/design/txt-format.md`
- `docs/design/verification.md`
- Lộ trình học và giải thích code: `docs/study-guide.md`
