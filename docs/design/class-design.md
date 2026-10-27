# Thiết kế lớp và OOP

Xem sơ đồ đầy đủ tại [Class, Service và Repository](../diagrams/classes/index.html).

## 1. Các tầng chính

| Tầng | Trách nhiệm |
|---|---|
| Entity | Giữ trạng thái và kiểm tra giá trị nội tại |
| Repository | Sở hữu entity trên RAM và tìm theo ID |
| Service | Kiểm tra quy tắc liên quan nhiều entity |
| ApplicationData | Sở hữu repository, nạp, kiểm tra và lưu dữ liệu |
| MainWindow | Nhận thao tác Qt, gọi service và hiển thị kết quả |

Widget không xử lý nghiệp vụ; service không đọc/ghi TXT trực tiếp.

## 2. Quan hệ giữa entity

Dự án dùng **direct association** bằng con trỏ không sở hữu:

```text
Seat      -> Auditorium
Screening -> Film, Auditorium
Ticket    -> Screening, Seat
```

Ví dụ `Ticket` giữ `const Screening*` và `const Seat*`. Các hàm
`screeningId()` và `seatId()` lấy ID từ đối tượng liên kết để ghi TXT.

Repository sở hữu entity; entity chỉ mượn địa chỉ. Vì vậy repository đích phải
sống lâu hơn đối tượng đang trỏ tới nó. `ApplicationData` tạo repository theo
thứ tự và sống đến khi cửa sổ đóng.

## 3. Entity

Mỗi entity có một `.h` và `.cpp` riêng. Thuộc tính là `private`; code ngoài đọc
qua getter. Constructor từ chối dữ liệu sai nên một entity đã tạo thành công luôn
hợp lệ về giá trị nội tại.

Entity không tự kiểm tra sự tồn tại của bản ghi liên quan. Ví dụ constructor
`Screening` nhận `Film&` và `Auditorium&`; service hoặc bước nạp chịu trách nhiệm
tìm đúng đối tượng trước khi gọi constructor.

## 4. Repository

`Repository<T>` dùng:

```text
LinkedList<T>             sở hữu entity
HashTable<EntityId, T*>   trỏ tới entity trong list
highestId_                sinh ID tiếp theo
dirty_                    báo dữ liệu chưa lưu
```

`FilmRepository`, `AuditoriumRepository`, `SeatRepository` và
`ScreeningRepository` kế thừa `Repository<T>` vì dùng nguyên API chung.

`TicketRepository` dùng composition với `Repository<Ticket>`. Nó chỉ công khai
tra cứu, thêm và duyệt; không công khai sửa/xóa vé đã bán. Hàm `relinkSeats()`
chỉ phục vụ nối lại `Seat*` khi kho ghế được thay.

Repository bị cấm copy/move vì bảng băm giữ địa chỉ node trong linked list.
Copy mặc định có thể làm index trỏ về node của repository cũ.

## 5. Service

| Service | Repository sử dụng | Nhiệm vụ |
|---|---|---|
| `CatalogService` | phim, phòng, ghế, suất, vé | CRUD phim/phòng, sinh ghế, chặn dữ liệu đang dùng |
| `ScreeningService` | phim, phòng, suất, vé | Kiểm tra thời gian, trùng lịch và suất đã có vé |
| `TicketService` | ghế, suất, vé | Xem trước, bán, tra cứu, sơ đồ ghế và doanh thu |

Service giữ reference, không sở hữu repository. `main.cpp` tạo `ApplicationData`
trước, sau đó truyền các repository vào service và truyền service vào `MainWindow`.

## 6. Thay đổi phòng trong direct association

Sửa phòng có thể tạo lại kho ghế, nên địa chỉ các `Seat` thay đổi. Luồng xử lý:

1. Dựng `SeatRepository` tạm.
2. Cập nhật phòng và gắn ghế mới với đối tượng phòng thật.
3. `TicketRepository::relinkSeats()` đổi con trỏ ghế trên các vé còn hợp lệ.
4. `SeatRepository::replaceWith()` thay kho ghế.

Không copy toàn bộ vé; dữ liệu vé và trạng thái dirty của vé được giữ nguyên.

## 7. OOP được sử dụng

- **Đóng gói:** thuộc tính private, thao tác qua phương thức.
- **Kế thừa:** repository cụ thể kế thừa `Repository<T>`.
- **Composition:** `TicketRepository` chứa `Repository<Ticket>`.
- **Association:** entity giữ con trỏ const đến entity liên quan.
- **Template:** cấu trúc dữ liệu và repository dùng lại cho nhiều kiểu.
- **Đa hình:** chỉ xuất hiện ở Qt qua lớp cơ sở như `QMainWindow`; entity không
  cần hệ thống virtual vì không có hành vi đa hình nghiệp vụ.

Thiết kế ưu tiên lớp nhỏ và quan hệ rõ ràng; không thêm interface, factory hoặc
callback khi bài toán không cần.
