# Thiết kế dữ liệu

Tài liệu mô tả dữ liệu logic, khóa, quan hệ và chuẩn hóa. Định dạng vật lý nằm
trong [thiết kế TXT](txt-format.md); cách biểu diễn con trỏ trên RAM nằm trong
[thiết kế lớp](class-design.md).

## 1. Lược đồ quan hệ

[ERD năm bảng](../diagrams/data-erd/README.md) thể hiện đủ cột, khóa và bội số;
không phải sơ đồ con trỏ của entity trên RAM.

```text
Film(film_id PK, title, duration_minutes)
Auditorium(auditorium_id PK, name, row_count, seats_per_row)
Seat(seat_id PK, auditorium_id FK, row_number, seat_number)
Screening(screening_id PK, film_id FK, auditorium_id FK,
          start_time, ticket_price)
Ticket(ticket_id PK, screening_id FK, seat_id FK, sold_at, unit_price)
```

## 2. Thuộc tính

ID và số nguyên dùng miền `int64_t`; tiền là VND nguyên. Thời điểm có độ chính
xác tới phút. Tên là chuỗi UTF-8; ràng buộc ký tự và cách mã hóa xem thiết kế TXT.

Riêng trên RAM, Film có thêm `normalizedTitle_` dẫn xuất để tìm kiếm; không phải
cột lưu trữ của lược đồ bên dưới. Xem [thiết kế index tên](film-name-search.md).

| Bảng | Thuộc tính ngoài khóa chính | Ý nghĩa |
|---|---|---|
| Film | `title`, `duration_minutes` | Tên và thời lượng tính bằng phút |
| Auditorium | `name`, `row_count`, `seats_per_row` | Tên, số hàng và số ghế mỗi hàng |
| Seat | `auditorium_id`, `row_number`, `seat_number` | Phòng và vị trí ghế, đánh số từ 1 |
| Screening | `film_id`, `auditorium_id`, `start_time`, `ticket_price` | Phim, phòng, giờ bắt đầu và giá suất |
| Ticket | `screening_id`, `seat_id`, `sold_at`, `unit_price` | Suất, ghế, thời điểm bán và giá đã bán |

## 3. Quan hệ và toàn vẹn

- Film → Screening, Auditorium → Seat/Screening, Screening → Ticket,
  Seat → Ticket đều là quan hệ một-nhiều; phía cha có thể chưa có bản ghi con.
- Mỗi Seat thuộc đúng một Auditorium; mỗi Screening có đúng một Film và
  Auditorium; mỗi Ticket có đúng một Screening và Seat. FK đều bắt buộc.
- Một Seat có thể xuất hiện trên vé của nhiều suất khác nhau.

| Dữ liệu | Ràng buộc cấu trúc |
|---|---|
| Mọi bảng | PK dương, duy nhất; không đổi ID khi cập nhật |
| Giá trị số | Số hàng/cột, thời lượng, giá và vị trí ghế dương; phép tính không tràn miền số |
| Seat | Tổ hợp `(auditorium_id, row_number, seat_number)` duy nhất và trong kích thước phòng |
| Auditorium | Có đủ `row_count × seats_per_row` ghế |
| Screening | Phim và phòng được tham chiếu phải tồn tại |
| Ticket | Suất và ghế phải tồn tại; phòng của ghế khớp phòng của suất |
| Ticket | Tổ hợp `(screening_id, seat_id)` duy nhất |

Tên không thay thế PK/FK. Chính sách tên duy nhất, xếp lịch và xóa nằm trong
[Business Rules Catalog](../requirements/business-rules.md). Hiệu ứng xóa tương
ứng RESTRICT/CASCADE do ứng dụng thực hiện, không phải constraint của DBMS.
TXT không tự có `FOREIGN KEY`, `UNIQUE` hoặc transaction.

## 4. Phụ thuộc hàm và chuẩn hóa

```text
film_id       -> title, duration_minutes
auditorium_id -> name, row_count, seats_per_row
seat_id       -> auditorium_id, row_number, seat_number
screening_id  -> film_id, auditorium_id, start_time, ticket_price
ticket_id     -> screening_id, seat_id, sold_at, unit_price
```

Các thuộc tính mô tả trực tiếp bản ghi; lược đồ được thiết kế theo 3NF trong
phạm vi đồ án. Những khóa ứng viên theo vị trí ghế hoặc cặp suất/ghế không làm
thay đổi việc dùng ID làm khóa chính. Tên phim/phòng không lặp trong Ticket.

`unit_price` là dữ liệu lịch sử, không suy ra từ giá suất hiện tại. Seat có PK
riêng để vé tham chiếu một vị trí ổn định giữa các lần chạy.

## 5. Dữ liệu suy ra

Không lưu riêng sức chứa phòng, giờ kết thúc, trạng thái ghế theo suất hoặc
doanh thu ngày. Công thức nghiệp vụ xem BR-S04, BR-T03 và BR-T06; các cấu trúc
kết quả trên RAM xem [thiết kế service](service-design.md).

Nguồn và phạm vi rút gọn của hình xem [mục lục sơ đồ](../diagrams/README.md).
