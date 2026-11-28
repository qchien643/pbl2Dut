# Bộ dữ liệu mẫu lớn

Sinh bằng `tools/generate_sample_data.py`, dữ liệu giả lập, không chứa người thật.
Giữ nguyên bộ nhỏ `data/demo`; bộ này được mở bằng:

```bash
./build/cinema data/sample
```

| Dữ liệu | Số lượng |
|---|---:|
| Phim | 1.000 |
| Phòng | 20 |
| Ghế | 2.000 |
| Suất chiếu | 2.000 |
| Vé | 5.000 |
| Người dùng giả lập trong fixtures | 1.000 |

**Ứng dụng chưa có entity User.** `fixtures/users.csv` và `fixtures/ticket_users.csv`
chỉ là dữ liệu phụ liên kết khách mô phỏng với vé, không được chương trình nạp,
không phải tài khoản đăng nhập. Không thêm bảng/cột mới vào năm TXT của ứng dụng.

- Tên phim gồm tên tiếng Việt, tiếng Anh, tên dài và nhiều từ/trigram dùng chung;
  mã chương làm tên duy nhất. Đây không phải 1.000 tác phẩm có thật.
- Ví dụ tìm trên tab Phim: `nguyen trai`, `trai nguyen`, `nguy`, `guy`, `nguyn trai`.
- Vé bán ngày **28/11/2026**; chọn ngày này ở tab Vé và doanh thu để thấy 5.000 vé.
- Suất bắt đầu từ **29/11/2026**, các suất cùng phòng cách nhau ít nhất bốn giờ,
  đủ thời lượng phim tối đa 160 phút và 15 phút nghỉ. Không trùng ghế trong cùng suất.
- Tab Bán vé mặc định hôm nay và chỉ hiện suất tương lai; bỏ lọc ngày hoặc chọn
  đúng ngày chiếu. Khi thời gian thực đi qua lịch mẫu, suất cũ không còn bán được.
- Mở trực tiếp rồi lưu/thoát sẽ ghi lại bộ này. Nên copy năm TXT sang thư mục tạm
  nếu muốn thử chỉnh sửa mà giữ nguyên fixture cho test và benchmark.

Tái tạo vào **thư mục mới**, script từ chối ghi đè thư mục đã có:

```bash
python3 tools/generate_sample_data.py /tmp/pbl2-sample-new
python3 tests/check_sample_data.py
./build/FilmSearchBenchmark data/sample 200
```

Quy mô và nguồn sinh xem `manifest.json`; kết quả đo xem
[benchmark](../../docs/testing/film-search-benchmark.md).
