# Định dạng tệp TXT

Đây là bản mô tả được dựng lại từ mô hình dữ liệu ban đầu. Mỗi dòng biểu diễn một bản ghi; các trường phân cách bằng dấu `|`, theo đúng thứ tự dưới đây. Giá trị văn bản không được chứa ký tự phân cách nếu chưa có quy tắc escape.

## Tên file và trường

| Tệp | Trường theo thứ tự |
|---|---|
| `films.txt` | `film_id|title|duration_minutes` |
| `auditoriums.txt` | `auditorium_id|name|row_count|seats_per_row` |
| `seats.txt` | `seat_id|auditorium_id|row_number|seat_number` |
| `screenings.txt` | `screening_id|film_id|auditorium_id|start_time|ticket_price` |
| `tickets.txt` | `ticket_id|screening_id|seat_id|sold_at|unit_price` |

ID, thời lượng, số hàng, số ghế và giá là số nguyên thập phân. ID, số đo và giá phải dương. Thời gian dùng biểu diễn ISO 8601 mà Qt `QDateTime` có thể đọc, ví dụ `2026-10-24T09:01:00.000`.

## Ví dụ

```text
# films.txt
1|Space Story|120

# auditoriums.txt
1|Room A|2|3

# seats.txt
1|1|1|1
2|1|1|2

# screenings.txt
1|1|1|2026-10-24T10:00:00.000|99000

# tickets.txt
1|1|1|2026-10-24T09:01:00.000|99000
```

Các dòng trống và dòng chú thích bắt đầu bằng `#` có thể được bỏ qua. Không chấp nhận bản ghi thiếu/thừa trường, dữ liệu số không hợp lệ, ID trùng, tham chiếu mồ côi hoặc bán trùng một ghế trong cùng suất.

## Nạp và lưu an toàn

- Nạp vào trạng thái tạm, kiểm tra toàn bộ bản ghi và quan hệ trước khi thay dữ liệu đang dùng.
- Nếu bất kỳ file/dòng nào sai, báo lỗi kèm tên file và số dòng; giữ nguyên dữ liệu hiện tại.
- Khi lưu, kiểm tra lỗi mở/ghi/đóng file; không báo thành công nếu một file chưa được ghi hoàn tất.
- Tệp TXT phù hợp bài tập offline nhỏ, không cung cấp giao dịch hay khóa ghi như DBMS.
