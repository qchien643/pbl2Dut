# ĐỊNH DẠNG TXT VÀ VÒNG ĐỜI DỮ LIỆU

**Cập nhật:** 26/10/2026. Đọc cùng [thiết kế dữ liệu](data-design.md) và
[hướng dẫn vấn đáp](../study-guide.md).

Định dạng được triển khai tại `src/persistence/TxtPersistence.{h,cpp}`; quy trình nạp,
kiểm tra và lưu toàn bộ được điều phối bởi `src/ApplicationData.cpp`.

## 1. Bộ file

Năm file nằm trong thư mục truyền cho `ApplicationData`. Thư mục phải tồn tại và
đọc được; chương trình không tự tạo thư mục dữ liệu hoặc lấy file từ thư mục khác.

| File | Header bắt buộc |
|---|---|
| `films.txt` | `film_id\|title\|duration_minutes` |
| `auditoriums.txt` | `auditorium_id\|name\|row_count\|seats_per_row` |
| `seats.txt` | `seat_id\|auditorium_id\|row_number\|seat_number` |
| `screenings.txt` | `screening_id\|film_id\|auditorium_id\|start_time\|ticket_price` |
| `tickets.txt` | `ticket_id\|screening_id\|seat_id\|sold_at\|unit_price` |

Trong file thực tế, dấu phân cách là `|`, không có dấu `\` trước nó.

- UTF-8 hợp lệ; đọc chấp nhận BOM ở đầu file và kết thúc dòng LF/CRLF. Ghi UTF-8
    không BOM, kết thúc dòng LF. Dòng cuối khi đọc không bắt buộc có newline.
- Mỗi dòng sau header là một bản ghi; không hỗ trợ dòng trống, comment hoặc escape.
    Số trường phải đúng, không tự bỏ qua dòng sai. File rỗng hợp lệ phải có header.
- Số là số nguyên thập phân trong miền `int64`, không có khoảng trắng, dấu `+` hoặc
    ký tự dư; các thuộc tính hiện tại đều phải dương. Tiền tính bằng VND.
- Tên được chuẩn hóa bỏ khoảng trắng ASCII đầu/cuối theo entity; từ chối tên rỗng,
    ký tự điều khiển và `|`. Tên tiếng Việt được giữ nguyên sau bước chuẩn hóa.
- Ngày giờ dùng `YYYY-MM-DD HH:mm`; các giới hạn lịch giống service.
- Chỉ ghi thuộc tính entity, không ghi con trỏ, chỉ mục, sức chứa, doanh thu hoặc
    trạng thái ghế. Giữ nguyên mã ghế khi nạp; không sinh lại sơ đồ từ kích thước.
- Mã kế tiếp được dựng từ mã lớn nhất còn trong file. Trong cùng phiên không tái
    dùng mã đã xóa; không lưu bộ đếm mã của các bản ghi đã xóa qua các phiên.

Ví dụ `films.txt`:

```text
film_id|title|duration_minutes
1|Phim Việt|120
```

## 2. Nạp và kiểm tra

`ApplicationData::load()` chỉ dùng trên đối tượng mới, repository rỗng. Nạp toàn bộ
vào dữ liệu tạm, kiểm tra xong mới hoán đổi sang năm repository chính. Nếu lỗi,
repository chính vẫn rỗng, không được tạo service/UI thao tác và `save()` bị chặn.

Kiểm tra gồm: UTF-8, header/trường/kiểu/ID trùng; khóa ngoại; vị trí và số lượng ghế;
lịch phòng gồm 15 phút nghỉ; cặp suất-ghế duy nhất; ghế đúng phòng và bán trước giờ
chiếu. Chấp nhận suất lịch sử. Giá vé lịch sử chỉ cần dương, không ép bằng giá suất
hiện tại. Dùng lại quy tắc lịch của service, nhưng không áp dụng điều kiện tương lai.

Lỗi bản ghi có tên file và dòng đang xét. Lỗi liên bản ghi có thể xuất hiện khi
đối chiếu dữ liệu của dòng khác, nên số dòng không luôn chỉ ra nguyên nhân duy
nhất. Khi kiểm tra RAM trước lưu, dòng chỉ vị trí bản ghi trong bản xuất hiện tại,
không nhất thiết là dòng cũ trên ổ đĩa.

Nếu thiếu một phần bộ file hoặc file không phải regular file (kể cả symlink), báo lỗi.
Nếu chưa có cả năm file và không có dấu vết lưu/phục hồi, trả
`LoadResult::NeedsEmptyConfirmation`; UI hỏi người dùng rồi mới gọi `load(true)`.
Chỉ sau xác nhận mới `isReady() == true`; chưa ghi file ở bước này.

### Các cờ trong ApplicationData

| Thành viên/hàm | Nghĩa | Không được hiểu là |
|---|---|---|
| `ready_` | Đã nạp/kiểm tra hoặc chấp nhận phiên rỗng | Đã lưu thành công |
| `hadFiles_` | Phiên có bộ file cũ cần sao lưu/phục hồi | Tự phát hiện mọi sửa đổi từ bên ngoài |
| `ownsStage_` | Đối tượng đã tạo thư mục staging, có trách nhiệm xử lý nó | Có thể xóa mọi thư mục `.cms-save` gặp trên đĩa |
| `recoveryRequired_` | Phục hồi lần trước thất bại, phải xử lý trước khi lưu tiếp | RAM mới đã mất |
| `isDirty()` | OR cờ dirty của năm repository | So sánh nội dung RAM với file |

Repository cung cấp `markSaved()` nhưng không ghi file. Chỉ lớp điều phối gọi
sau nạp hợp lệ hoặc sau khi xuất đủ bộ thành công. `save()` không dùng dirty để
quyết định chỉ ghi một số file.

## 3. Lưu toàn bộ và phục hồi

`save()` chỉ dùng sau `load()` thành công và luôn xuất cả năm file, kể cả không có
thay đổi. Destructor không tự lưu; lỗi lưu ném exception, UI phải giữ cửa sổ mở.

1. Kiểm tra toàn bộ RAM và sự hiện diện của bộ file.
2. Tạo riêng thư mục `.cms-save/new` và `.cms-save/old` trong thư mục dữ liệu.
3. Xuất đủ năm file mới, kiểm tra lỗi ghi, flush và close.
4. Nếu có bộ cũ, sao chép đủ năm file vào `old` trước khi thay bất kỳ file chính nào.
5. Rename từng file mới thành file chính. Sau khi hoàn tất, đổi tên `.cms-save`
    thành `.cms-backup-N` chưa tồn tại; chỉ lúc này mới xóa cờ chưa lưu.

Lỗi giữa chừng: phục hồi các file đã thay từ bản sao, không tiêu thụ bản sao; lần
lưu đầu thì gỡ những file chính vừa tạo. RAM và cờ chưa lưu giữ nguyên, thông báo
nêu kết quả phục hồi. Khi phục hồi thành công, có thể sửa nguyên nhân rồi gọi lại
`save()` trên cùng đối tượng; chỉ dọn file tạm thuộc lần lưu của chính đối tượng.

Nếu phục hồi thất bại, `needsManualRecovery()` trả `true`, chặn lưu tiếp. Sau khi
sửa nguyên nhân I/O, có thể gọi `retryRecovery()` để thử phục hồi đủ bộ cũ trong
cùng phiên, vẫn giữ RAM mới; khi thành công mới thử `save()` lại.

Các thư mục `.cms-backup-N` được giữ nguyên, không tự xóa hoặc ghi đè. Sau khi kiểm
tra bộ chính đã nạp được, người vận hành có thể lưu trữ/dọn các bản sao cũ. Bản sao
của lần lưu đầu có thư mục `old` rỗng vì trước đó chưa có dữ liệu.

## 4. Giới hạn và phục hồi sau dừng bất thường

Chỉ hỗ trợ một phiên, không sửa file từ ngoài trong lúc chạy. Không có journal,
fsync hoặc bảo đảm nguyên tử cả bộ khi crash/mất điện; thay đổi chưa lưu có thể mất.
Kiểm tra file trước lưu chỉ phát hiện thiếu/thừa hoặc loại file không phù hợp,
không so checksum để phát hiện nội dung đã bị người khác sửa.
Không bảo đảm phục hồi nếu runtime không thể tiếp tục (ví dụ cạn bộ nhớ trong
hàm `noexcept` của thư viện hệ thống); đây cũng là dừng bất thường.

Nếu lần mở sau thấy `.cms-save`, chương trình dừng nạp và không ghi đè. Người vận
hành cần giữ bản sao hiện trạng để kiểm tra: nếu `old` có đủ năm file hợp lệ thì
phục hồi **cả bộ**, không ghép file cũ/mới. Chỉ chuyển `.cms-save` sang nơi lưu trữ
sau khi đã xử lý; sau đó mở lại để kiểm tra toàn vẹn. Nếu `old` thiếu, cần kiểm tra
bộ chính hoặc một `.cms-backup-N/old` đầy đủ; không tự suy luận rồi xóa dữ liệu.

Nếu cả bộ chính mất nhưng vẫn có `.cms-backup-*`, không cho xác nhận phiên rỗng.
Nếu lỗi không phục hồi được trong phiên, giữ cửa sổ/RAM để xử lý. Khung Qt hiện tại
bắt exception khi đóng, giữ cửa sổ mở và thử `retryRecovery()` nếu cần ở lần lưu lại.

## 5. Câu hỏi cần tự lần theo code

- Vì sao codec `load()` thêm vào repository tạm, không thêm thẳng vào bộ chính?
- Vì sao cần kiểm tra cả `flush()` lẫn `close()` sau vòng ghi?
- Nếu file thứ ba thay thất bại, `installed` đang bằng mấy và phục hồi những file nào?
- Nếu lỗi trước khi thay bất kỳ file nào, vì sao không cần phục hồi file chính?
- Vì sao `retryRecovery()` giữ nguyên RAM và chưa được gọi `markSaved()`?
- Vì sao nhiều lần rename không tạo ra một transaction nguyên tử cho cả năm file?

Cách học: vẽ trạng thái `RAM / new / old / file chính` ở từng bước, rồi đối chiếu
`testSaveFailures` và `testFailedRecoveryAndValidation` trong
`tests/ApplicationDataTest.cpp`. Không thử lỗi phá hủy trên dữ liệu đang dùng.
