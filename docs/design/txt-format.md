# Định dạng TXT và vòng đời dữ liệu

## 1. Bộ file

| File | Header |
|---|---|
| `films.txt` | `film_id|title|duration_minutes` |
| `auditoriums.txt` | `auditorium_id|name|row_count|seats_per_row` |
| `seats.txt` | `seat_id|auditorium_id|row_number|seat_number` |
| `screenings.txt` | `screening_id|film_id|auditorium_id|start_time|ticket_price` |
| `tickets.txt` | `ticket_id|screening_id|seat_id|sold_at|unit_price` |

File dùng UTF-8, mỗi bản ghi một dòng, phân cách bằng `|`. Chuỗi không được
chứa `|`, xuống dòng hoặc ký tự điều khiển. Thời gian có dạng
`YYYY-MM-DD HH:mm`; tiền là số nguyên VND.

Header ở bảng trên là thứ tự chuẩn khi ghi. Khi đọc, các cột được phép đổi vị
trí nếu header và dữ liệu đổi tương ứng; không được thiếu, thêm hoặc trùng tên cột.

## Chuyển đổi model và TXT

`TxtPersistence::readTable()` đọc một file thành `std::vector<Row>`.
`Row` là `std::map<std::string, std::string>`: tên cột → chuỗi giá trị.
Đây chỉ là dữ liệu tạm khi mở; entity trong phiên vẫn do linked list của
repository sở hữu.

`persistence/ModelIO.cpp` chứa header chuẩn và các hàm đọc/ghi tường minh cho
năm model. Không có mapping template, `tuple`, `std::apply` hoặc reflection.

```cpp
const std::vector<txt_persistence::Row> rows =
    txt_persistence::readTable(path, txt_persistence::seatHeader);
for (const txt_persistence::Row& row : rows) {
    Seat seat = txt_persistence::readSeat(row, data.rooms);
    if (!data.seats.insert(seat))
        throw std::invalid_argument("Duplicate primary key");
}
```

- `readTable`: kiểm tra UTF-8, header, số trường và ghép tên cột với giá trị.
- `readSeat`: lấy từng trường bằng `row.at("tên_cột")`, chuyển số, tìm phòng
  qua `IRepository<Auditorium>`, kiểm tra tồn tại rồi gọi constructor.
- `writeSeat`: gọi getter và ghi các cột ngăn bằng `|`, kết thúc bằng xuống dòng.
- `ApplicationData`: nạp theo thứ tự phụ thuộc, gọi `insert`; khi lưu duyệt từng
  repository và gọi hàm ghi tương ứng. Lỗi đọc báo tên file và số dòng.

Thứ tự constructor và thứ tự ghi được viết rõ trong code; đổi vị trí thuộc tính
private trong `.h` không ảnh hưởng. Đổi định dạng thì sửa header, hàm đọc và hàm
ghi tương ứng. Chấp nhận vài dòng lặp cho năm model để dễ đọc và vấn đáp.

## 2. Nạp dữ liệu

Thứ tự nạp:

```text
Film -> Auditorium -> Seat -> Screening -> Ticket
```

Thứ tự này cho phép tìm đối tượng cha và tạo direct association khi đọc FK.
Dữ liệu được nạp vào `ApplicationData` tạm, sau đó `validate()` kiểm tra khóa,
quan hệ, sơ đồ ghế, lịch chiếu và vé. Chỉ khi toàn bộ hợp lệ, repository chính
mới được thay.

Ứng dụng từ chối:

- thiếu một phần bộ file;
- header, số trường, số nguyên hoặc UTF-8 sai;
- ID trùng hoặc quan hệ không tồn tại;
- dữ liệu vi phạm quy tắc nghiệp vụ.

Thư mục hoàn toàn rỗng được phép bắt đầu phiên mới sau khi người dùng xác nhận.

## 3. Dữ liệu trong phiên

Mọi thao tác chỉ thay đổi repository trên RAM. `dirty_` cho biết repository có
thay đổi chưa lưu; nó không phải trạng thái đã bán của ghế.

Đọc cả file một lần lúc mở, không đọc/ghi file theo từng thao tác nghiệp vụ.

Đóng ứng dụng bằng `Lưu và thoát` mới gọi `ApplicationData::save()`.

## 4. Quy trình lưu

1. Kiểm tra lại toàn bộ dữ liệu.
2. Ghi năm file mới vào `.cms-save/new`.
3. Sao chép file cũ vào `.cms-save/old`.
4. Đổi tên file mới thành file chính.
5. Chuyển thư mục staging thành `.cms-backup-N`.
6. Đánh dấu repository đã lưu.

`openOutput()` bật exception khi ghi lỗi; gọi `close()` tường minh để phát hiện
cả lỗi đẩy dữ liệu đệm ra file. Chỉ thay file chính sau khi đã đóng đủ năm file mới.

Nếu lỗi xảy ra, chương trình cố phục hồi file cũ. Cửa sổ và dữ liệu RAM vẫn
được giữ để người dùng sửa nguyên nhân và thử lại.

Nếu rollback không hoàn tất, `needsManualRecovery()` chặn lần lưu tiếp theo cho
đến khi phục hồi thành công. Khi khởi động, `.cms-save` còn tồn tại được xem là
dấu hiệu một lần lưu chưa hoàn tất và chương trình không tự ghi đè.

## 5. Giới hạn

- Không chạy hai phiên trên cùng thư mục dữ liệu.
- Không sửa TXT bên ngoài khi ứng dụng đang mở.
- Thay đổi chưa lưu có thể mất nếu tiến trình bị buộc dừng.
- `.cms-backup-N` là bản sao phục hồi, không tự động xóa.
