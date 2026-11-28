# Thiết kế lớp và OOP

Tài liệu mô tả trách nhiệm lớp, dependency và quyền sở hữu/vòng đời đối tượng.
Chi tiết hàm service xem [thiết kế service](service-design.md); chính sách rạp
xem [Business Rules Catalog](../requirements/business-rules.md).

Sơ đồ hiện hành: [kiến trúc](../diagrams/system-architecture/README.md),
[entity](../diagrams/entities-uml/README.md) và
[repository–service, DI/DIP](../diagrams/repository-service-uml/README.md).

## 1. Các tầng chính

| Tầng | Trách nhiệm |
|---|---|
| Entity | Giữ trạng thái và kiểm tra giá trị nội tại |
| Repository | Sở hữu entity trên RAM, index và các truy vấn dữ liệu |
| Search | Thuật toán scan, trigram và khớp text dùng lại; không sở hữu entity |
| Service | Kiểm tra quy tắc liên quan nhiều entity |
| Helper | Thực hiện phép xử lý kỹ thuật do service gọi; không quyết định chính sách rạp |
| CinemaDatabase | Sở hữu repository, nạp, kiểm tra và lưu dữ liệu |
| CinemaWindow | Nhận thao tác Qt, gọi service và hiển thị kết quả |

Widget không xử lý nghiệp vụ; service không đọc/ghi TXT trực tiếp.

`helper/DateTimeHelper` chỉ kiểm tra định dạng/lịch, chuyển chuỗi ↔ phút và cộng
phút an toàn; không biết entity hay repository. Service tự lấy thời lượng phim
để tính giờ kết thúc. Ranh giới kiểm tra khi nạp/lưu nằm trong
[thiết kế TXT](txt-format.md).

`DateTimeHelper` cũng cung cấp `localMinuteNow()` và kiểu con trỏ hàm `Clock`.
Service nhận `date_time::Clock` qua constructor để dùng giờ thật hoặc đồng hồ giả
trong test; đây là helper, không phải service nghiệp vụ.

`helper/SeatMap` nhận kích thước và danh sách ghế để dựng ma trận, không truy vấn
repository và cho phép ô trống. CatalogService sở hữu quy tắc phòng đầy đủ;
TicketService gọi hàm static `CatalogService::makeRoomSeatMap()` để dùng lại quy tắc,
rồi đánh dấu ghế đã bán theo suất. Không có `SeatMapService` hay đối tượng helper
được inject; helper được gọi trực tiếp. Constructor các service không đổi.

## 2. Quan hệ giữa entity

Xem [UML năm entity](../diagrams/entities-uml/README.md) để đối chiếu thuộc tính con
trỏ, hướng association và bội số với code.

Dự án dùng **direct association** bằng con trỏ không sở hữu:

```text
Seat      -> Auditorium
Screening -> Film, Auditorium
Ticket    -> Screening, Seat
```

Ví dụ `Ticket` giữ `const Screening*` và `const Seat*`. Các hàm
`screeningId()` và `seatId()` lấy ID từ đối tượng liên kết để ghi TXT.

Repository sở hữu entity; entity chỉ mượn địa chỉ. Vì vậy repository đích phải
sống lâu hơn đối tượng đang trỏ tới nó. `CinemaDatabase` tạo repository theo
thứ tự và sống đến khi cửa sổ đóng.

## 3. Entity

Mỗi entity có một `.h` và `.cpp` riêng. Thuộc tính là `private`; code ngoài đọc
qua getter. Constructor từ chối dữ liệu sai nên một entity đã tạo thành công luôn
hợp lệ về giá trị nội tại.

Constructor dùng `helper/FieldValidation`: `requirePositiveNumber()` từ chối số
không dương; `validateAndTrimName()` kiểm tra tên và bỏ khoảng trắng đầu/cuối.
Đây là helper kiểm tra trường dữ liệu, không phải entity hay service.

`Film` giữ thêm `normalizedTitle_` (UTF-32) ngay trong đối tượng, dẫn xuất từ tên
gốc bằng helper `TextNormalization` khi dựng. Getter chỉ đọc; copy/swap giữ cả hai trường đồng bộ.
Trường này phục vụ tìm kiếm và không ghi vào TXT, không phải association mới.

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

`FilmRepository`, `AuditoriumRepository`, `SeatRepository`, `ScreeningRepository`
và `TicketRepository` đều kế thừa `Repository<T>`, dùng cùng CRUD và cùng
interface `IRepository<T>`. Không cần interface riêng cho vé.

Repository quản lý dữ liệu, ID và chỉ mục; không quyết định được phép sửa/xóa
vé hay không. `TicketService` không cung cấp hàm sửa/xóa vé, UI chỉ gọi service.
Gọi CRUD trực tiếp trên repository không thay thế kiểm tra nghiệp vụ.
`TicketRepository` chỉ bổ sung hàm private `relinkSeats()` để nối lại `Seat*`
khi kho ghế được thay; đây là xử lý kỹ thuật, không đổi nội dung vé hay dirty.

Repository bị cấm copy/move vì bảng băm giữ địa chỉ node trong linked list.
Copy mặc định có thể làm index trỏ về node của repository cũ.

`FilmRepository` triển khai `IFilmRepository`, kế thừa CRUD của `IRepository<Film>`
và bổ sung `searchTitle()` trong cùng hợp đồng. Kho giữ
index trigram → Film* và bộ trigram theo ID để đồng bộ CRUD. Tìm và scan cùng
đọc `Film::normalizedTitle()`, không chuẩn hóa lại tên hay tra ID ứng viên.
Repository sở hữu `TrigramIndex<Film>` từ `search/`; template nhận nội dung cột
khi ghi, không phụ thuộc Film. `TextSearch` chứa thuật toán text dùng chung;
`sequentialSearch()` nhận điều kiện kiểm tra cột. Helper chỉ chuẩn hóa text.
`searchTitleSequential()` là đối chứng cho test/benchmark, không là Strategy.
CatalogService nhận một `IFilmRepository&`, không truyền kho phim hai lần.
`Repository<T>` và `IFilmRepository` dùng kế thừa ảo (`public virtual`) tới
`IRepository`: hai đường kế thừa của FilmRepository dùng chung một interface
CRUD, tránh chuyển kiểu mơ hồ hoặc viết lại hàm CRUD. Đây không phải hai kho;
linked list và hash index ID vẫn chỉ nằm trong `Repository<Film>`.
Quy ước tại ranh giới service–repository: entity có interface repository chuyên
biệt thì mọi service dùng interface đó, kể cả khi chỉ cần CRUD. Với phim,
CatalogService nhận `IFilmRepository&`, ScreeningService nhận
`const IFilmRepository&`. Entity chưa có hợp đồng riêng vẫn dùng `IRepository<T>`;
không tạo interface riêng chỉ để đổi tên. Hàm đọc file dùng chung vẫn có thể
nhận hợp đồng CRUD nền; quy ước này áp dụng cho dependency của service. Chi tiết:
[index tên](film-name-search.md).

## 5. Service

[UML DI/DIP](../diagrams/repository-service-uml/README.md) minh họa đường phụ thuộc
của TicketService và chuỗi kế thừa kho vé; bảng dưới đây liệt kê cả ba service.

Chính sách nằm trong [Business Rules Catalog](../requirements/business-rules.md).
Hành vi từng hàm, các hàm chỉ tra cứu và ánh xạ BR sang code nằm trong
[thiết kế service](service-design.md). Phần này chỉ mô tả dependency và vòng đời.

| Service | Repository sử dụng | Nhiệm vụ |
|---|---|---|
| `CatalogService` | phim, phòng, ghế, suất; `IRoomStorage` | Điều phối danh mục và phòng/ghế |
| `ScreeningService` | phim, phòng, suất, vé | Điều phối suất và dùng helper thời gian |
| `TicketService` | ghế, suất, vé | Xem trước, bán, tra cứu, sơ đồ ghế và doanh thu |

Service giữ reference tới interface repository và không sở hữu repository.
Các service dùng `IFilmRepository` cho kho phim, `const` khi chỉ đọc; các kho
chưa có interface chuyên biệt dùng `IRepository<T>`. `IRoomStorage` lưu phòng cùng bộ ghế như một thao tác
RAM; service không tự dựng repository cụ thể.

Đây là **constructor injection**: `main.cpp` tạo `CinemaDatabase` trước, rồi
truyền repository vào constructor của service và truyền service vào `CinemaWindow`.

```text
CinemaDatabase sở hữu FilmRepository, SeatRepository, ..., RoomStorage
              ↓ main.cpp truyền vào constructor
Service giữ IRepository<Seat>&, IRepository<Ticket>&, ...
CatalogService giữ IFilmRepository& và IRoomStorage&
ScreeningService giữ const IFilmRepository& vì chỉ đọc kho phim
```

Đây là **DIP tại ranh giới service–repository**: nghiệp vụ phụ thuộc hợp đồng,
không phụ thuộc cách cài đặt linked list/hash table. `list()` trả danh sách con
trỏ chỉ đọc tới entity gốc, không trả iterator của kho và không copy entity.
Không dùng các con trỏ này sau khi bản ghi bị xóa hoặc kho bị thay.

Phạm vi có chủ đích cho môn học: không thêm interface cho entity, Qt, service
hay toàn bộ TXT. `CinemaDatabase` vẫn biết lớp cụ thể để quản lý vòng đời và
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

- Service kiểm tra các BR-D trước khi xóa; sau khi được phép, repository
  giải phóng đúng entity. Thứ tự gọi được mô tả trong thiết kế service.
- Khi xóa phòng không dựng lại toàn bộ kho ghế: ghế của phòng khác giữ nguyên
  địa chỉ, nên vé đang trỏ tới chúng không cần nối lại.
- Service quyết định API thao tác; repository vẫn hỗ trợ CRUD chung, không
  tự suy ra chính sách từ loại entity.
- `Repository::erase()` chỉ xóa chỉ mục và node, đánh dấu dirty; không tự kiểm
  tra nghiệp vụ hay tự đặt con trỏ liên quan thành `nullptr`.

Giao diện xác nhận rồi gọi service; `CinemaDatabase::save()` mới ghi trạng
thái RAM ra TXT. Phải đi qua service để không tạo con trỏ treo.

## 7. OOP được sử dụng

- **Đóng gói:** thuộc tính private, thao tác qua phương thức.
- **Kế thừa:** repository cụ thể kế thừa `Repository<T>`.
- **Composition:** `Repository<T>` chứa linked list và hash table.
- **Association:** entity giữ con trỏ const đến entity liên quan.
- **Template:** cấu trúc dữ liệu và repository dùng lại cho nhiều kiểu.
- **Đa hình:** service gọi hàm ảo qua `IRepository<T>`; Qt
  cũng dùng đa hình qua các lớp cơ sở như `QMainWindow`.
- **Dependency Injection:** `main.cpp` truyền dependency qua constructor.
- **Dependency Inversion:** service phụ thuộc interface; repository cụ thể
  triển khai interface đó bằng `override`.

`DependencyInjectionTest` dùng implementation dựa trên `std::list`, không kế
thừa `Repository<T>`, để chạy ba service mà không sửa code nghiệp vụ.
Kho vé trong test cũng dùng đúng template này, không cần implementation riêng.
Kho phim test triển khai IFilmRepository bằng CRUD và scan chuỗi con để kiểm tra DI;
quy tắc text đầy đủ và index được kiểm tra riêng trong FilmSearchTest/SearchTest.
