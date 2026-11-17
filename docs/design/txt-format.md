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

## Ánh xạ model và TXT

`persistence/ModelMappings.h` khai báo một ánh xạ cho mỗi model. Mỗi cột gồm tên,
getter khi ghi và quy tắc đọc: `integerColumn`, `textColumn` hoặc `referenceColumn`.
Thứ tự khai báo ánh xạ phải khớp tham số constructor, không phụ thuộc thứ tự
thuộc tính trong `.h` hay thứ tự cột của file đầu vào.

```cpp
const auto columns = txt_persistence::seatMapping(data.rooms);
txt_persistence::load(path, data.seats, columns);
txt_persistence::save(path, data.seats, columns);
```

- `Mapping.h`: ghép tên cột với vị trí trong header, chuyển kiểu và gọi constructor.
- `TxtPersistence::load()`: đọc dòng, kiểm tra dữ liệu và thêm model vào repository.
- `TxtPersistence::save()`: sinh header và ghi giá trị qua getter từ cùng ánh xạ.
- `referenceColumn`: khi đọc dùng `findById()` rồi trả reference tới đối tượng
  trong `IRepository<T>` được truyền vào; khi ghi gọi getter lấy ID, không lưu
  địa chỉ con trỏ.

Ánh xạ chỉ mượn repository; repository phải còn sống khi đọc. Không còn hàm
`parse()` hay `writeRecord()` riêng cho từng model. C++17 dùng `std::tuple`,
`std::apply` và parameter pack để truyền các kết quả đọc đúng kiểu vào constructor;
validation và thuộc tính private của model được giữ nguyên.

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

Đóng ứng dụng bằng `Lưu và thoát` mới gọi `ApplicationData::save()`.

## 4. Quy trình lưu

1. Kiểm tra lại toàn bộ dữ liệu.
2. Ghi năm file mới vào `.cms-save/new`.
3. Sao chép file cũ vào `.cms-save/old`.
4. Đổi tên file mới thành file chính.
5. Chuyển thư mục staging thành `.cms-backup-N`.
6. Đánh dấu repository đã lưu.

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
