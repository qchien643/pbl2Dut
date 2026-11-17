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

`IRepository<T>` là interface thuần ảo: `nextId`, `findById`, `insert`,
`update`, `erase`, `list`. `Repository<T>` triển khai interface này bằng:

```text
LinkedList<T>             sở hữu entity
HashTable<EntityId, T*>   trỏ tới entity trong list
highestId_                sinh ID tiếp theo
dirty_                    báo dữ liệu chưa lưu
```

`FilmRepository`, `AuditoriumRepository`, `SeatRepository` và
`ScreeningRepository` kế thừa `Repository<T>` vì dùng nguyên API chung.

`TicketRepository` triển khai `ITicketRepository` và dùng composition với
`Repository<Ticket>`. Interface vé chỉ cho phép lấy ID, tìm, thêm và lấy danh sách;
không có sửa/xóa. Lớp cụ thể chỉ công khai
tra cứu, thêm và duyệt; không công khai sửa/xóa vé đã bán. Hàm `relinkSeats()`
chỉ phục vụ nối lại `Seat*` khi kho ghế được thay.

Repository bị cấm copy/move vì bảng băm giữ địa chỉ node trong linked list.
Copy mặc định có thể làm index trỏ về node của repository cũ.

## 5. Service

| Service | Repository sử dụng | Nhiệm vụ |
|---|---|---|
| `CatalogService` | phim, phòng, ghế, suất; `IRoomStorage` | CRUD phim/phòng, sinh ghế, chặn dữ liệu đang dùng |
| `ScreeningService` | phim, phòng, suất, vé | Kiểm tra thời gian, trùng lịch và suất đã có vé |
| `TicketService` | ghế, suất, vé | Xem trước, bán, tra cứu, sơ đồ ghế và doanh thu |

Service giữ reference tới `IRepository<T>`, `ITicketRepository` và không sở hữu
repository. `CatalogService` nhận thêm `IRoomStorage` để lưu phòng cùng bộ ghế
như một thao tác RAM; không tự dựng repository cụ thể.

Đây là **constructor injection**: `main.cpp` tạo `ApplicationData` trước, rồi
truyền repository vào constructor của service và truyền service vào `MainWindow`.

```text
ApplicationData sở hữu FilmRepository, SeatRepository, ..., RoomStorage
              ↓ main.cpp truyền vào constructor
Service giữ IRepository<Film>&, IRepository<Seat>&, ITicketRepository&
CatalogService còn giữ IRoomStorage&
```

Đây là **DIP tại ranh giới service–repository**: nghiệp vụ phụ thuộc hợp đồng,
không phụ thuộc cách cài đặt linked list/hash table. `list()` trả danh sách con
trỏ chỉ đọc tới entity gốc, không trả iterator của kho và không copy entity.
Không dùng các con trỏ này sau khi bản ghi bị xóa hoặc kho bị thay.

Phạm vi có chủ đích cho môn học: không thêm interface cho entity, Qt, service
hay toàn bộ TXT. `ApplicationData` vẫn biết lớp cụ thể để quản lý vòng đời và
nạp/lưu; `RoomStorage` biết lớp cụ thể để thay kho và nối lại con trỏ an toàn.
Không dùng factory hoặc DI container.

## 6. Thay đổi phòng trong direct association

Service kiểm tra phòng chưa có suất và sinh danh sách ghế mới; sau đó gọi
`IRoomStorage`. `RoomStorage` thực hiện phần lưu bên dưới:

1. Dựng `Repository<Seat>` tạm, gồm ghế phòng khác và bộ ghế mới.
2. Cập nhật phòng và gắn ghế mới với đối tượng phòng thật.
3. `TicketRepository::relinkSeats()` đổi con trỏ ghế trên các vé còn hợp lệ.
4. `Repository<Seat>::replaceWith()` thay kho ghế.

Không copy toàn bộ vé; dữ liệu vé và trạng thái dirty của vé được giữ nguyên.
Mọi cấp phát hoàn tất trước khi đổi phòng/kho chính; lỗi chuẩn bị giữ nguyên RAM.

### Xóa và bảo toàn association

- `CatalogService::eraseFilm()` chặn phim còn suất; `eraseRoom()` chặn phòng
  còn suất, sau đó xóa từng ghế của phòng rồi xóa phòng.
- Khi xóa phòng không dựng lại toàn bộ kho ghế: ghế của phòng khác giữ nguyên
  địa chỉ, nên vé đang trỏ tới chúng không cần nối lại.
- `ScreeningService::erase()` chặn suất đã có vé. Vé không có API xóa công khai.
- `Repository::erase()` chỉ xóa chỉ mục và node, đánh dấu dirty; không tự kiểm
  tra nghiệp vụ hay tự đặt con trỏ liên quan thành `nullptr`.

Giao diện xác nhận rồi gọi service; `ApplicationData::save()` mới ghi trạng
thái RAM ra TXT. Phải đi qua service để không tạo con trỏ treo.

## 7. OOP được sử dụng

- **Đóng gói:** thuộc tính private, thao tác qua phương thức.
- **Kế thừa:** repository cụ thể kế thừa `Repository<T>`.
- **Composition:** `TicketRepository` chứa `Repository<Ticket>`.
- **Association:** entity giữ con trỏ const đến entity liên quan.
- **Template:** cấu trúc dữ liệu và repository dùng lại cho nhiều kiểu.
- **Đa hình:** service gọi hàm ảo qua `IRepository<T>`/`ITicketRepository`; Qt
  cũng dùng đa hình qua các lớp cơ sở như `QMainWindow`.
- **Dependency Injection:** `main.cpp` truyền dependency qua constructor.
- **Dependency Inversion:** service phụ thuộc interface; repository cụ thể
  triển khai interface đó bằng `override`.

`DependencyInjectionTest` dùng implementation dựa trên `std::list`, không kế
thừa `Repository<T>`, để chạy ba service mà không sửa code nghiệp vụ.
