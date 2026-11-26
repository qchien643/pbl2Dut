# Đặc tả yêu cầu phần mềm

## 1. Tổng quan

| Mục | Nội dung |
|---|---|
| Sản phẩm | Ứng dụng quản lý rạp chiếu phim |
| Công nghệ | C++17, Qt 6 Widgets, CMake |
| Người dùng | Một nhân viên vận hành |
| Lưu trữ | TXT UTF-8, nạp khi mở và lưu khi thoát |
| Môi trường | Offline, một máy, một phiên chạy |

Mục tiêu là quản lý phim, phòng, suất chiếu và vé; đồng thời minh họa OOP, thiết
kế dữ liệu và cấu trúc dữ liệu tự cài đặt.

Ngoài phạm vi: tài khoản, đặt vé online, giữ/hủy vé, thanh toán điện tử, nhiều
chi nhánh, nhiều phiên đồng thời, khuyến mãi và DBMS.

## 2. Yêu cầu chức năng

| Nhóm | Yêu cầu |
|---|---|
| Phim | Thêm, sửa, xóa, liệt kê và tìm theo mã hoặc tên. Không xóa phim đã có suất. |
| Phòng | Thêm, sửa, xóa và xem sơ đồ ghế. Ghế được sinh theo số hàng và số ghế mỗi hàng. |
| Suất chiếu | Thêm, sửa, xóa và lọc theo ngày, phim, phòng. Không cho trùng lịch trong cùng phòng. |
| Bán vé | Chọn suất chưa bắt đầu, chọn ghế trống, xem trước và xác nhận đã nhận tiền mặt. |
| Vé và doanh thu | Tìm vé theo mã; liệt kê vé và tính doanh thu theo ngày bán. |
| Dữ liệu | Nạp và kiểm tra toàn vẹn khi mở; lưu đồng bộ năm file khi người dùng chọn Lưu và thoát. Database không kiểm tra lại trùng lịch. |

## 3. Ràng buộc nghiệp vụ

Quản lý lịch chiếu, bán vé và xóa dữ liệu phải tuân theo
[Danh mục quy tắc nghiệp vụ](business-rules.md). Tài liệu đó ghi từng quy tắc
đã có trong code, mã BR và hàm thực hiện; SRS không lặp lại mô tả cài đặt.

## 4. Mô hình dữ liệu

| Entity | Thuộc tính chính |
|---|---|
| `Film` | `film_id`, `title`, `duration_minutes` |
| `Auditorium` | `auditorium_id`, `name`, `row_count`, `seats_per_row` |
| `Seat` | `seat_id`, `auditorium_id`, `row_number`, `seat_number` |
| `Screening` | `screening_id`, `film_id`, `auditorium_id`, `start_time`, `ticket_price` |
| `Ticket` | `ticket_id`, `screening_id`, `seat_id`, `sold_at`, `unit_price` |

Sức chứa, giờ kết thúc, trạng thái ghế và doanh thu là dữ liệu tính toán, không
lưu thành entity riêng.

## 5. Giao diện

Ứng dụng có năm tab: `Bán vé`, `Phim`, `Phòng & ghế`, `Suất chiếu`, `Vé và doanh thu`.

- Lỗi nhập phải được báo rõ và không làm thay đổi dữ liệu một phần.
- Sơ đồ ghế phân biệt ghế trống, đã bán và đang chọn.
- Trước khi bán phải hiển thị phim, phòng, suất, ghế và giá.
- Thanh trạng thái cho biết dữ liệu đã được lưu hay chưa.

## 6. Yêu cầu chất lượng

- Chạy hoàn toàn offline và không dùng container STL/Qt làm kho entity chính.
- Dùng `LinkedList`, `HashTable` và `Matrix2D` tự cài đặt.
- Kiểm tra tràn số khi tính ID, sức chứa và doanh thu.
- Lỗi lưu phải giữ dữ liệu trên RAM và giữ cửa sổ mở để thử lại.
- Sau khi lưu và mở lại, dữ liệu và báo cáo phải không đổi.

## 7. Tiêu chí nghiệm thu

1. CRUD phim, phòng và suất hợp lệ; dữ liệu sai bị từ chối.
2. Phòng tạo đúng số ghế; lịch chiếu không chồng lấn.
3. Bán ghế trống tạo đúng một vé; bán lại cùng ghế bị từ chối.
4. Tra cứu vé và doanh thu đúng theo ngày bán.
5. Dữ liệu lưu được, mở lại được và giữ đúng quan hệ.
6. File thiếu, sai định dạng hoặc lưu lỗi không gây mất dữ liệu âm thầm.

Chi tiết triển khai nằm trong [thiết kế dữ liệu](../design/data-design.md),
[thiết kế lớp](../design/class-design.md) và [kiểm thử](../design/verification.md).
