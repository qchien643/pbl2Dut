# Hướng dẫn đọc code và vấn đáp

Tài liệu giúp học và tự giải thích code; không thay thế đặc tả hay thiết kế.
Đọc [SRS](../requirements/srs.md) và [Business Rules Catalog](../requirements/business-rules.md)
để hiểu bài toán trước.

## 1. Cách giải thích một dòng code

Khi thầy chỉ vào một dòng, trả lời:

1. Đây là giá trị, con trỏ, tham chiếu hay đối tượng?
2. Dòng này đọc/thay đổi trạng thái nào?
3. Ai sở hữu dữ liệu; địa chỉ còn hợp lệ đến khi nào?
4. Bỏ dòng này sẽ sai điều gì?
5. Lỗi trả bằng exception, `false` hay `nullptr`; dữ liệu cũ có còn nguyên không?

## 2. Lộ trình

| Chặng | Đọc code | Tự giải thích | Tài liệu đối chiếu |
|---|---|---|---|
| 1 | `entities/` | Constructor, getter, private và con trỏ liên kết | [Thiết kế lớp mục 2–3](../design/class-design.md#2-quan-hệ-giữa-entity) |
| 2 | `DataStructures/` | Node, bucket, iterator, chỉ số ma trận | Code LinkedList/HashTable/Matrix2D và ba test DSA |
| 3 | `repositories/` | Template, list/index, ID, dirty, override | [Thiết kế lớp mục 4](../design/class-design.md#4-repository) |
| 4 | `services/` | Phân biệt điều kiện nghiệp vụ, truy vấn và an toàn kỹ thuật | [Thiết kế service](../design/service-design.md) |
| 5 | `fileio/`, `database/` | Row → entity → repository; ghi và phục hồi | [Thiết kế TXT](../design/txt-format.md) |
| 6 | `main.cpp`, `gui/` | Ghép dependency, thời gian sống, signal/slot | [Thiết kế lớp mục 5](../design/class-design.md#5-service) |
| 7 | `tests/` | Chuẩn bị, thao tác, assert và trường hợp lỗi | [Kiểm thử](../testing/verification.md) |

## 3. Các điểm C++ cần nắm

### Entity và quan hệ

Với `const Auditorium* auditorium_`, hãy phân biệt địa chỉ phòng được mượn
với đối tượng Seat đang sở hữu thuộc tính con trỏ đó. Con trỏ không làm Seat
sở hữu hoặc tự giải phóng Auditorium. ID vẫn cần cho PK/FK khi lưu file.

Bài tập: mở Seat, Screening, Ticket; chỉ ra từng thuộc tính thể hiện association
và getter lấy ID từ đối tượng được tham chiếu.

### Cấu trúc dữ liệu

- LinkedList: Node chứa `T value`; `pushBack()` trả reference tới giá trị mới.
- HashTable: bucket chứa entry; va chạm được xử lý bằng duyệt và so khóa.
- Matrix2D: ô `(row, column)` có chỉ số `row * columns + column`; dữ liệu ghế
  đánh số từ 1, chỉ số ma trận bắt đầu từ 0.
- Range-for dùng `begin()/end()`; không tự chọn danh sách chỉ vì class có một
  thuộc tính LinkedList.

Bài tập: lần theo một lần `insert()` từ service đến list và index; giải thích
`T&`, `T*`, `T**`, `*record` và vì sao phải rollback khi index thất bại.

### Repository

Giải thích vì sao index chứa địa chỉ node làm copy mặc định nguy hiểm.
Với `update()`, phân biệt copy entity với copy repository; tìm đoạn copy trước
và swap giữ địa chỉ bản ghi. Với `list()`, danh sách sở hữu node chứa con trỏ,
không sở hữu entity gốc. Xóa bản ghi hoặc thay kho có thể làm địa chỉ cũ mất hiệu lực.

Bài tập: giải thích `dirty_` khác trạng thái ghế đã bán và vì sao TicketRepository
vẫn có CRUD dù TicketService không cung cấp sửa/xóa vé.

### DI và DIP

```cpp
IFilmRepository& films_; // CatalogService: CRUD và tìm tên trong một hợp đồng
const IFilmRepository& films_; // ScreeningService: cùng hợp đồng, chỉ đọc
```

DI là truyền dependency từ ngoài vào; DIP là phụ thuộc interface thay vì cách
lưu cụ thể. Chỉ truyền `FilmRepository&` đã có DI nhưng chưa đạt DIP ở ranh giới này.
Quy ước dự án: nếu entity có interface repository chuyên biệt, mọi service dùng
interface đó; chỉ thay đổi `const` theo quyền đọc/ghi, không chọn interface theo
việc service có gọi search hay chưa. Các kho chưa có interface riêng dùng `IRepository<T>`.

Bài tập: mở main.cpp và chỉ ra nơi tạo CinemaDatabase, service, cửa sổ. Theo chuỗi
`FilmRepository -> IFilmRepository -> IRepository<Film>` để giải thích vì sao
truyền đối tượng cụ thể vào tham số interface được. FilmRepository còn kế thừa
Repository<Film> để dùng lại storage/CRUD; hai nhánh dùng chung base IRepository
nhờ kế thừa ảo. Mở DependencyInjectionTest
để tìm implementation thay thế.

## 4. Bài tập lần theo luồng

Bản đồ trực quan: [kiến trúc](../diagrams/system-architecture/README.md) →
[entity](../diagrams/entities-uml/README.md) → [DI/DIP](../diagrams/repository-service-uml/README.md) →
[TXT/RAM](../diagrams/txt-lifecycle/README.md) → [bán vé](../diagrams/ticket-sale-sequence/README.md).
Đối chiếu [ERD](../diagrams/data-erd/README.md) với UML: FK trên file không phải địa chỉ
con trỏ. Dùng [sửa/xóa phòng](../diagrams/room-change-flow/README.md) để luyện về vòng đời.

Dùng [thiết kế service](../design/service-design.md) làm bản đồ, nhưng giải thích
từ source thay vì học thuộc tài liệu:

1. Thêm/sửa phim: BR-C04 được kiểm tra sau bước nào? Vì sao bỏ qua cùng ID?
   Khi đổi tên phim đã có vé, liên kết và giá vé có thay đổi không?
2. Sửa phòng: chỉ ra service kiểm tra BR, RoomStorage chuẩn bị kho tạm,
   nơi gắn lại phòng và nối lại con trỏ ghế trên vé.
3. Tạo suất: chỉ ra thời lượng phim được lấy ở đâu; giải thích điều kiện hai
   khoảng giao nhau và khác biệt giữa giờ kết thúc với giờ trả phòng.
4. Bán vé: tại sao xem trước chưa đủ? Điều gì ngăn hai vé cùng suất/ghế?
5. Xóa phòng: vì sao xóa từng ghế phòng đó không làm vé của phòng khác bị treo?
6. Nạp TXT: `readTable()` ghép tên cột với chuỗi ở đâu; `readSeat()` tìm phòng ở
   đâu; đối tượng sau `insert()` thực sự được sở hữu bởi đâu?
7. Lưu: khi ghi lỗi, dữ liệu RAM, file chính và bản sao phục hồi còn trạng thái nào?

## 5. Câu hỏi tự vấn đáp

1. Direct association khác lưu ID như thế nào?
2. Ai sở hữu Film, Seat, Screening, Ticket? Thứ tự hủy có ý nghĩa gì?
3. `const`, reference và pointer khác nhau thế nào trong getter/constructor?
4. Vì sao `findById()` gặp kiểu `T* const*`?
5. `= delete`, `virtual`, `= 0`, `override` có tác dụng gì?
6. Vì sao TicketRepository cần `friend` và `relinkSeats()`?
7. Vì sao có IRoomStorage dù đã có IRepository?
8. Helper thời gian khác service suất chiếu ở đâu?
9. Quy tắc nào kiểm tra ở entity, repository, service hoặc database?
10. Vì sao `find()` và `rooms()` có thể không có nghiệp vụ riêng mà vẫn cần thiết?
11. Quy tắc tên duy nhất có được database kiểm tra lại khi load không?
12. Vì sao thay vị trí thuộc tính private không tự đổi thứ tự cột TXT?
13. Test nào chứng minh từng BR? Một test Passed chưa chứng minh được điều gì?

## 6. Cách luyện

Chọn một hàm, giải thích từng dòng theo mục 1 rồi chạy test tương ứng trong
[tài liệu kiểm thử](../testing/verification.md). Thử thay đổi sai trên bản sao để
xem assert nào phát hiện. Sơ đồ chỉ là hình tham khảo, cần đối chiếu trạng thái
tại [mục lục sơ đồ](../diagrams/README.md).
