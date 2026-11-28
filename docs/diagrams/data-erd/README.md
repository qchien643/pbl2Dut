# ERD năm bảng

[Mở sơ đồ HTML](data-erd.html) · [Tất cả sơ đồ](../README.md)

## Mục đích

Mô tả mô hình dữ liệu logic: cột, khóa và lực lượng quan hệ, không phải con trỏ RAM.

## Cách đọc

- `#` đánh dấu PK, `→` đánh dấu FK; mọi FK đều bắt buộc.
- Film–Screening, Auditorium–Screening/Seat, Screening–Ticket và Seat–Ticket
  đều là một–nhiều; phía cha có thể chưa có bản ghi con.
- Vị trí `(auditorium_id, row_number, seat_number)` duy nhất trong Seat.
- Cặp `(screening_id, seat_id)` duy nhất trong Ticket; một ghế vẫn có thể bán
  ở các suất khác nhau. Ghế phải thuộc phòng của suất.
- TXT lưu ID khóa ngoại; khi nạp mới tra ID và tạo con trỏ trên RAM.

## Nguồn đối chiếu

[Thiết kế dữ liệu](../../design/data-design.md): miền dữ liệu, toàn vẹn, 3NF.
[EntityFile.cpp](../../../src/fileio/EntityFile.cpp): các cột thực tế khi đọc/ghi.
[CinemaDatabase.cpp](../../../src/database/CinemaDatabase.cpp): kiểm tra toàn vẹn.

## Giới hạn

Đủ năm bảng, tất cả cột và năm quan hệ. Không vẽ schema SQL: dự án không dùng
DBMS; FK/UNIQUE do code kiểm tra. Chính sách xóa chỉ định nghĩa tại
[Business Rules](../../requirements/business-rules.md#4-quy-ước-xóa-dữ-liệu).
Đối chiếu source ngày 27/11/2026.
