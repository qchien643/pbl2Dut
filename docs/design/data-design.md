# Thiết kế dữ liệu

## 1. Phạm vi

Dự án mô hình hóa dữ liệu theo quan hệ nhưng lưu bằng TXT, không dùng DBMS.
Khóa ngoại và ràng buộc được kiểm tra bằng C++ khi nạp hoặc thao tác.

Xem sơ đồ tương tác tại [ERD Archify](../diagrams/erd/index.html).

## 2. Lược đồ quan hệ

```text
Film(film_id PK, title, duration_minutes)
Auditorium(auditorium_id PK, name, row_count, seats_per_row)
Seat(seat_id PK, auditorium_id FK, row_number, seat_number)
Screening(screening_id PK, film_id FK, auditorium_id FK,
          start_time, ticket_price)
Ticket(ticket_id PK, screening_id FK, seat_id FK, sold_at, unit_price)
```

Quan hệ:

- Một `Film` có nhiều `Screening`.
- Một `Auditorium` có nhiều `Seat` và `Screening`.
- Một `Screening` có nhiều `Ticket`.
- Một `Seat` có thể xuất hiện trên nhiều vé của các suất khác nhau.

## 3. Khóa và ràng buộc

| Dữ liệu | Ràng buộc |
|---|---|
| Mọi entity | ID dương, duy nhất và không đổi |
| `Seat` | `(auditorium_id, row_number, seat_number)` duy nhất |
| `Screening` | Phim/phòng phải tồn tại; lịch cùng phòng không chồng lấn |
| `Ticket` | Suất/ghế phải tồn tại; ghế thuộc phòng của suất |
| `Ticket` | `(screening_id, seat_id)` duy nhất |

Phòng hợp lệ phải có đủ `row_count × seats_per_row` ghế. Số hàng, cột, thời
lượng, giá và ID đều phải dương.

## 4. Phụ thuộc hàm và chuẩn hóa

```text
film_id       -> title, duration_minutes
auditorium_id -> name, row_count, seats_per_row
seat_id       -> auditorium_id, row_number, seat_number
screening_id  -> film_id, auditorium_id, start_time, ticket_price
ticket_id     -> screening_id, seat_id, sold_at, unit_price
```

Mỗi bảng chỉ lưu thuộc tính phụ thuộc vào khóa của chính nó, nên lược đồ đạt
3NF trong phạm vi bài. Tên phim/phòng không lặp trong `Ticket`; khi tra cứu sẽ
đi qua `Screening` và `Seat`.

`unit_price` được lưu riêng có chủ đích để giữ giá đã bán, kể cả khi giá suất
thay đổi. `Seat` được lưu vì vé cần một mã ghế ổn định giữa các lần chạy.

## 5. Dữ liệu tính toán

Không lưu riêng:

- `capacity = row_count × seats_per_row`;
- giờ kết thúc = giờ bắt đầu + thời lượng phim;
- trạng thái ghế = có/không có vé của suất và ghế đó;
- doanh thu ngày = tổng `unit_price` của vé bán trong ngày.

## 6. Thực thi trong code

| Trách nhiệm | Vị trí |
|---|---|
| Kiểm tra giá trị một entity | Constructor trong `src/entities/` |
| ID, lưu bản ghi, chỉ mục | `Repository<T>` |
| Quan hệ và quy tắc nhiều entity | Các lớp trong `src/services/` |
| Kiểm tra toàn bộ tập dữ liệu | `ApplicationData::validate()` |
| Đọc/ghi định dạng | `TxtPersistence` |

`HashTable<EntityId, T*>` giúp tìm theo ID trung bình O(1). Các truy vấn lọc,
trùng lịch, ghế đã bán và doanh thu duyệt tuyến tính vì dữ liệu môn học nhỏ.

## 7. Tính nhất quán

- Nạp vào repository tạm, kiểm tra xong mới thay dữ liệu chính.
- Thao tác service chuẩn bị dữ liệu trước khi cập nhật repository.
- Lưu cả năm file theo một quy trình có bản sao phục hồi.

Đây là cách mô phỏng khóa ngoại và giao dịch ở tầng ứng dụng; TXT tự nó không
có `FOREIGN KEY`, `UNIQUE` hay transaction.
