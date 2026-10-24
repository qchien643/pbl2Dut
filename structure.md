# Cấu trúc dữ liệu — Hệ thống quản lý rạp chiếu phim

Tài liệu này chuyển các yêu cầu trong SRS-CMS-001 thành thiết kế dữ liệu cho ứng dụng C++17/Qt 6 chạy offline, một người vận hành và một phiên. Dữ liệu nghiệp vụ được giữ trong RAM; năm tệp TXT là nguồn lưu trữ bền vững duy nhất.

## 1. Nguyên tắc thiết kế

- Chỉ có năm entity nghiệp vụ: `Film`, `Auditorium`, `Seat`, `Screening` và `Ticket`.
- Dùng `LinkedList<T>`, `HashTable<Key, Value>` và `Matrix2D<T>` tự cài đặt làm cấu trúc lưu bản ghi/chỉ mục chính. Không dùng container STL/Qt làm nơi lưu entity hoặc chỉ mục chính.
- Các ID là số nguyên dương, do chương trình cấp, duy nhất trong phạm vi entity tương ứng và bất biến sau khi tạo.
- Mã tiền dùng số nguyên VND, không dùng số thực. Thời điểm dùng giờ địa phương, độ chính xác đến phút theo `YYYY-MM-DD HH:mm`.
- Dữ liệu tính được không lưu trùng: sức chứa, giờ kết thúc, trạng thái ghế theo suất và doanh thu được tính từ dữ liệu gốc.
- UI chỉ giữ trạng thái trình bày/tạm thời; mọi thay đổi nghiệp vụ đi qua service và repository.

## 2. Entity và trường dữ liệu

| Entity | Trường | Kiểu gợi ý | Ý nghĩa / ràng buộc |
|---|---|---|---|
| `Film` | `film_id` | `std::int64_t` | ID dương, duy nhất. |
| | `title` | `QString` | Bắt buộc, không rỗng sau khi trim. |
| | `duration_minutes` | `std::int32_t` | Số phút dương; chỉ sửa khi chưa có suất. |
| `Auditorium` | `auditorium_id` | `std::int64_t` | ID dương, duy nhất. |
| | `name` | `QString` | Bắt buộc, không rỗng sau khi trim. |
| | `row_count` | `std::int32_t` | Số hàng dương. |
| | `seats_per_row` | `std::int32_t` | Số ghế mỗi hàng dương. |
| `Seat` | `seat_id` | `std::int64_t` | ID dương, duy nhất. |
| | `auditorium_id` | `std::int64_t` | Phải tham chiếu phòng tồn tại. |
| | `row_number` | `std::int32_t` | Đánh số từ 1 đến `row_count`. |
| | `seat_number` | `std::int32_t` | Đánh số từ 1 đến `seats_per_row`. |
| `Screening` | `screening_id` | `std::int64_t` | ID dương, duy nhất. |
| | `film_id` | `std::int64_t` | Phải tham chiếu phim tồn tại. |
| | `auditorium_id` | `std::int64_t` | Phải tham chiếu phòng tồn tại. |
| | `start_time` | `QDateTime` | Giờ địa phương; phải hợp lệ và ở tương lai khi tạo/sửa. |
| | `ticket_price` | `std::int64_t` | Giá vé VND dương. |
| `Ticket` | `ticket_id` | `std::int64_t` | ID dương, duy nhất. Chỉ cấp khi hoàn tất bán. |
| | `screening_id` | `std::int64_t` | Phải tham chiếu suất tồn tại. |
| | `seat_id` | `std::int64_t` | Ghế phải thuộc phòng của suất. |
| | `sold_at` | `QDateTime` | Thời điểm xác nhận đã nhận tiền mặt. |
| | `unit_price` | `std::int64_t` | Giá VND được chốt tại lúc bán; không đọc lại giá hiện tại của suất. |

Các entity nên đóng gói dữ liệu và kiểm tra tính hợp lệ cục bộ. Kiểm tra liên kết giữa nhiều entity, lịch chồng lấn và tính duy nhất liên hợp thuộc service/repository.

## 3. Cấu trúc lưu trữ và chỉ mục trong RAM

Mỗi repository sở hữu một `LinkedList<Entity>` làm nơi giữ các bản ghi. Các `HashTable` bên dưới lưu khóa tra cứu tới ID (hoặc vị trí/handle ổn định do `LinkedList` cung cấp), không tạo bản sao entity.

| Repository | Danh sách bản ghi | Chỉ mục băm cần có |
|---|---|---|
| `FilmRepository` | `LinkedList<Film>` | `film_id → Film`; khóa tìm tên có thể chuẩn hóa/tìm tuyến tính vì chỉ cần tìm một phần tên. |
| `AuditoriumRepository` | `LinkedList<Auditorium>` | `auditorium_id → Auditorium`. |
| `SeatRepository` | `LinkedList<Seat>` | `seat_id → Seat`; `(auditorium_id, row_number, seat_number) → seat_id`; `auditorium_id → danh sách/đầu mối các ghế`. |
| `ScreeningRepository` | `LinkedList<Screening>` | `screening_id → Screening`; `auditorium_id → các suất của phòng`; có thể thêm `film_id → các suất của phim` và ngày → các suất nếu cần cho lọc. |
| `TicketRepository` | `LinkedList<Ticket>` | `ticket_id → Ticket`; `(screening_id, seat_id) → ticket_id` để chặn bán trùng; ngày bán → các vé trong ngày để báo cáo. |

Khóa ghép cần kiểu/hash chuyên biệt, chẳng hạn `ScreeningSeatKey{screening_id, seat_id}`. Khóa ngày bán lấy theo ngày địa phương của `sold_at`. Các chỉ mục theo nhóm có thể ánh xạ tới đầu mối danh sách liên kết phụ; phải cập nhật đồng bộ với danh sách chính khi thêm/xóa.

### Ma trận ghế phòng

Mỗi phòng có thể được trình bày/định vị bằng `Matrix2D<std::int64_t>` kích thước `row_count × seats_per_row`. Ô `(row_number - 1, seat_number - 1)` chứa `seat_id`; giá trị `0` dành cho ô không có ghế nếu cấu trúc cho phép khuyết vị trí. Với sơ đồ chữ nhật đầy đủ theo SRS, mọi ô phải chứa đúng một ID ghế.

Ma trận là cấu trúc định vị ghế theo phòng, không phải bản sao entity `Seat`. Khi sửa phòng chưa có suất, tạo lại toàn bộ ghế và ma trận như một thao tác nguyên tử trên RAM; khi xóa phòng chưa có suất, xóa ma trận và các ghế thuộc phòng cùng lúc.

## 4. Quan hệ và bất biến

- Một `Auditorium` có `row_count × seats_per_row` ghế. Phải kiểm tra phép nhân và cấp ID không tràn trước khi tạo.
- Mỗi vị trí `(auditorium_id, row_number, seat_number)` chỉ có một ghế.
- Một `Screening` tham chiếu đúng một phim và một phòng đang tồn tại.
- Giờ kết thúc suất là `start_time + Film.duration_minutes`; không cần lưu riêng. Khoảng chiếm dụng phòng là nửa mở `[start_time, end_time + 15 phút)`. Hai khoảng nửa mở không chồng lấn khi `A.start < B.end && B.start < A.end`.
- Vé chỉ được tạo trước `start_time`, sau xác nhận đã nhận tiền mặt; hủy xác nhận không làm thay đổi repository.
- Với mỗi cặp `(screening_id, seat_id)` tồn tại tối đa một vé. Cùng ghế có thể được bán ở suất khác.
- Phòng và phim chỉ được sửa/xóa khi không có suất tham chiếu; suất chỉ được sửa/xóa khi chưa có vé. Không xóa vé đã bán.
- Trạng thái ghế của một suất được suy ra từ các vé có `screening_id` tương ứng: trống nếu không có vé; đã bán nếu có vé. Ghế đang chọn chỉ là trạng thái tạm trong UI, không lưu vào entity hay TXT.
- Doanh thu ngày là tổng `Ticket.unit_price` theo ngày của `Ticket.sold_at`. Ngày không có vé cho số vé và doanh thu bằng 0.
- Mã vé, số lượng và tổng doanh thu phải được kiểm tra tràn số.

## 5. Quy trình bán vé POS

1. Lọc suất chưa bắt đầu theo ngày/phim/phòng; lấy thông tin phim, phòng, thời gian và giá từ các repository.
2. Chọn một ghế trong ma trận phòng; kiểm tra ghế thuộc đúng phòng và tra chỉ mục `(screening_id, seat_id)` để xác định còn trống.
3. Hiển thị thông tin xác nhận. Việc chọn/hủy ở bước này chỉ là trạng thái UI, chưa khóa ghế và chưa tạo vé.
4. Khi người vận hành xác nhận đã nhận tiền, service kiểm tra lại giờ bắt đầu, ghế/phòng, suất và tính duy nhất của cặp suất-ghế.
5. Tạo vé với `unit_price` chốt từ giá suất tại thời điểm xác nhận; thêm vé và cập nhật toàn bộ chỉ mục như một thao tác nguyên tử. Nếu một bước thất bại, không để lại vé hoặc chỉ mục bán dở dang.

Không có entity giỏ hàng, giao dịch thanh toán, khách hàng, giữ ghế hay hoàn vé vì các chức năng này nằm ngoài phạm vi SRS.

## 6. Nạp, lưu và nhất quán

- Ánh xạ dữ liệu lần lượt với `films.txt`, `auditoriums.txt`, `seats.txt`, `screenings.txt`, `tickets.txt`; định dạng dòng/phục hồi chi tiết thuộc `docs/design/txt-format.md`.
- Khi khởi động, đọc toàn bộ bộ file vào dữ liệu tạm, kiểm tra cú pháp, ID duy nhất, khóa ghép, tham chiếu, vị trí ghế, lịch phòng và vé. Chỉ đưa vào `ApplicationData` và cho phép chỉnh sửa nếu toàn bộ dữ liệu hợp lệ.
- Nếu bộ file chưa tồn tại, chỉ cho khởi tạo phiên rỗng sau xác nhận. File thiếu/hỏng/không đọc được không được xem như dữ liệu rỗng và không được ghi đè.
- Trong phiên, mọi thao tác chỉ cập nhật RAM; không tự ghi sau mỗi thao tác.
- `Lưu và thoát` và đóng cửa sổ dùng chung một quy trình lưu đủ năm file. Chỉ đóng sau khi lưu thành công; khi lỗi, giữ nguyên RAM và cửa sổ để người vận hành thử lại.
- Việc thêm/xóa entity phải cập nhật danh sách chính và mọi chỉ mục liên quan như một đơn vị. Dùng kiểm tra trước và cơ chế rollback/cập nhật tạm để không công bố trạng thái nửa chừng.
- Chỉ mục là dữ liệu dẫn xuất trong RAM; có thể dựng lại từ entity khi nạp. Không lưu chỉ mục ra TXT.

## 7. Dữ liệu không tạo thành entity riêng

Không cần entity/repository riêng cho sức chứa, thời gian kết thúc, trạng thái ghế, doanh thu, ngày báo cáo, lựa chọn tạm trên UI hoặc trạng thái chưa lưu. Chúng được tính từ entity gốc hoặc quản lý ở lớp UI/service. Thiết kế này giữ đúng phạm vi SRS và tránh dữ liệu trùng có thể lệch nhau.
