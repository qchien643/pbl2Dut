# Hướng dẫn đọc code và vấn đáp

## 1. Cách giải thích một dòng code

Khi thầy chỉ vào một dòng, trả lời theo năm ý:

1. Kiểu dữ liệu là gì: giá trị, con trỏ, tham chiếu hay đối tượng?
2. Dòng này đọc hoặc thay đổi trạng thái nào?
3. Ai sở hữu dữ liệu và địa chỉ còn hợp lệ đến khi nào?
4. Vì sao cần dòng này; bỏ đi sẽ sai điều gì?
5. Lỗi được báo bằng exception, `false` hay `nullptr`; trạng thái cũ có còn không?

## 2. Lộ trình đọc file

| Chặng | File | Cần hiểu |
|---|---|---|
| 1 | `entities/Film`, `Auditorium` | Đóng gói, constructor, getter, validation |
| 2 | `entities/Seat`, `Screening`, `Ticket` | Direct association và con trỏ không sở hữu |
| 3 | `DataStructures/LinkedList.h` | Node, ownership, iterator, thêm/xóa |
| 4 | `HashTable.h`, `Matrix2D.h` | Va chạm, rehash, ánh xạ hai chiều |
| 5 | `repositories/Repository.h` | Template, list/index, ID, dirty, rollback |
| 6 | Các repository cụ thể | Kế thừa và composition của `TicketRepository` |
| 7 | `CatalogService` | CRUD phim/phòng và tạo lại ghế |
| 8 | `ScreeningService` | Thời gian, trùng lịch và suất có vé |
| 9 | `TicketService` | Xem trước, bán, tra cứu và doanh thu |
| 10 | `TxtPersistence`, `ApplicationData` | Nạp, validate, save và recovery |
| 11 | `main.cpp`, `MainWindow` | Thứ tự sống, signal/slot và luồng Qt |
| 12 | `tests/` | Cách kiểm chứng hành vi và lỗi |

## 3. Entity và association

- Thuộc tính private, đọc qua getter.
- Constructor đảm bảo giá trị nội tại hợp lệ.
- `Seat` giữ `const Auditorium*`.
- `Screening` giữ `const Film*` và `const Auditorium*`.
- `Ticket` giữ `const Screening*` và `const Seat*`.

Con trỏ trên không sở hữu bộ nhớ. Repository sở hữu entity; vì vậy repository
đích phải sống lâu hơn entity đang liên kết với nó.

ID vẫn tồn tại trong file TXT. Khi nạp, `ApplicationData` dùng `findById()` để
tìm đối tượng thật rồi truyền reference vào constructor.

## 4. Cấu trúc dữ liệu

### LinkedList

- Mỗi `Node` sở hữu một `T value`.
- `pushBack()` trả reference tới giá trị trong node mới.
- `erase()` nối lại hai phía rồi giải phóng đúng node.
- Iterator chỉ giữ con trỏ node hiện tại, không copy danh sách.

### HashTable

- Mỗi bucket là một linked list entry.
- Va chạm được xử lý bằng cách duyệt bucket và so sánh khóa.
- Repository lưu `T*`, nên `find()` có thể trả `T**`.
- Rehash dựng bảng mới trước; chỉ thay bảng cũ khi thành công.

### Matrix2D

Dữ liệu nằm trong một mảng một chiều; ô `(row, column)` có chỉ số
`row * columns + column`. Entity dùng hàng/cột từ 1, còn ma trận dùng chỉ số từ 0.

## 5. Repository

`Repository<T>` sở hữu entity trong `LinkedList<T>` và dùng
`HashTable<EntityId, T*>` làm chỉ mục.

- `findById()` trả con trỏ chỉ đọc, không copy entity.
- `insert()` phải cập nhật thành công cả list và index.
- `update()` copy trước rồi swap để lỗi copy không làm hỏng bản ghi cũ.
- `replaceWith()` thay list và index cùng lúc để con trỏ vẫn đúng.
- `dirty_` cho biết repository đã thay đổi sau lần lưu gần nhất.

Copy/move bị cấm vì index chứa địa chỉ node của chính repository. Copy mặc định
có thể tạo list mới nhưng giữ con trỏ tới list cũ.

`TicketRepository` không override `Repository`; nó **chứa** một
`Repository<Ticket>` và chuyển tiếp các hàm được phép. Cách này giấu `update()`
và `erase()` khỏi API công khai.

## 6. Ba luồng service cần thuộc

### Sửa phòng

```text
MainWindow -> CatalogService
           -> kiểm tra phòng chưa có suất
           -> dựng SeatRepository tạm
           -> cập nhật phòng
           -> nối lại Seat* trên vé
           -> thay kho ghế
```

### Tạo suất

```text
MainWindow -> ScreeningService
           -> tìm Film và Auditorium
           -> tạo Screening candidate
           -> kiểm tra tương lai và trùng lịch
           -> insert ScreeningRepository
```

Khoảng sử dụng phòng là `[bắt đầu, kết thúc + 15 phút)`. Hai suất đúng ranh
giới được phép; hai khoảng giao nhau thì bị từ chối.

### Bán vé

```text
MainWindow -> TicketService::preview()
           -> xác nhận đã nhận tiền
           -> TicketService::sell()
           -> kiểm tra lại suất và ghế
           -> tạo Ticket với giá chốt
           -> insert TicketRepository
```

`preview()` không giữ ghế và không tạo vé. `sell()` luôn kiểm tra lại để tránh
dùng kết quả xem trước đã cũ.

## 7. Nạp và lưu

Khi mở, `ApplicationData` nạp năm file theo thứ tự phụ thuộc, dựng direct
association và kiểm tra toàn bộ. Dữ liệu chính chỉ được thay khi bước này thành công.

Khi lưu, chương trình ghi file tạm, sao lưu file cũ rồi đổi tên. Nếu lỗi, RAM và
cửa sổ được giữ để thử lại.

## 8. Qt và main

Thứ tự tạo trong `main.cpp`:

```text
QApplication
ApplicationData
CatalogService / ScreeningService / TicketService
MainWindow
app.exec()
```

Thứ tự này bảo đảm repository sống lâu hơn service và entity liên kết. Qt parent
sở hữu widget con; các service chỉ là reference trong `MainWindow`.

## 9. Câu hỏi tự vấn đáp

1. Direct association khác lưu ID như thế nào?
2. Ai sở hữu `Film`, `Seat`, `Screening` và `Ticket`?
3. Vì sao `Repository` cấm copy/move?
4. Vì sao `findById()` gặp kiểu `T* const*`?
5. `dirty_` có ý nghĩa gì?
6. Vì sao `TicketRepository` dùng composition?
7. Vì sao sửa phòng phải nối lại con trỏ ghế trên vé?
8. Vì sao doanh thu dùng `Ticket::unitPrice()`?
9. Vì sao `preview()` chưa đủ để bán vé?
10. Khi lưu file thứ ba thất bại, chương trình giữ dữ liệu bằng cách nào?
11. Quy tắc nào thuộc entity, service và `ApplicationData`?
12. Test nào chứng minh bán trùng ghế bị từ chối?

## 10. Cách luyện

1. Vẽ lại [luồng hệ thống](diagrams/workflow/index.html).
2. Vẽ năm entity và các direct association không nhìn tài liệu.
3. Chọn một hàm service và giải thích từng dòng theo năm câu ở mục 1.
4. Chạy test tương ứng rồi cố tình sửa sai trên một bản sao để xem test nào bắt được.
