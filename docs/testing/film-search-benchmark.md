# Benchmark index tìm tên phim

Đo ngày **28/11/2026**, Release C++17, GCC 15.2.0, Linux x86_64,
Intel Core i5-1135G7. Số đo trên máy phát triển, không phải SLA.

## 1. Hai phương pháp được so sánh

Tên được chuẩn hóa một lần trong constructor và lưu ngay ở
`Film::normalizedTitle_`, không có bảng tên phụ.

| Phương pháp | Cách tìm |
|---|---|
| Trigram index | Tra posting list chứa `Film*`, hợp/giao ứng viên rồi xác nhận bằng tên chuẩn hóa |
| Sequential scan | Duyệt linked list Film, xác nhận bằng cùng cột tên chuẩn hóa |

Cả hai gọi repository (`searchTitle()` và `searchTitleSequential()`), dùng
**cùng hàm `text_search::matches()`**, chuẩn hóa câu tìm trong mỗi lần đo,
thu kết quả vào `LinkedList<const Film*>`, chuyển sang vector ID và sắp xếp.
Không chuẩn hóa lại tên phim hoặc tra ID khi xác nhận ứng viên. Chương trình
bắt buộc hai cách trả cùng tập ID trước khi đo.

Thời gian query không gồm nạp TXT, xây index, chuẩn hóa tên phim, vẽ Qt hoặc
copy Film của service. Chi phí khởi tạo báo riêng bên dưới.

## 2. Dữ liệu và cách chạy

[Bộ mẫu](../../data/sample/README.md): 1.000 phim, 20 phòng, 2.000 ghế,
2.000 suất, 5.000 vé. CSV phụ chứa 1.000 người dùng giả lập, không phải entity
được ứng dụng nạp. Tên sinh từ 20 tên gốc, hậu tố và mã chương; nhiều từ dùng
chung, không đại diện toàn bộ dữ liệu thực tế. Bộ 10.000 phim sinh cùng quy tắc
ở thư mục tạm, các bảng còn lại giữ nguyên số lượng.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_GUI=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/FilmSearchBenchmark data/sample 1000
# Chọn đường dẫn thư mục mới; bộ sinh từ chối ghi đè dữ liệu cũ.
python3 tools/generate_sample_data.py /tmp/pbl2-benchmark-new --films 10000
./build/FilmSearchBenchmark /tmp/pbl2-benchmark-new 200
```

Nguồn: [FilmSearchBenchmark.cpp](../../tools/FilmSearchBenchmark.cpp).
Warm-up mỗi phương pháp 5 lần, sau đó lấy trung bình 1.000 lần với 1.000 phim,
200 lần với 10.000 phim; dùng `steady_clock`, đơn vị microsecond/query.
Đo lần lượt, không pin CPU/tắt turbo; kết quả có thể dao động khi chạy lại.
Số liệu dưới đây đã đo lại sau khi tách `TrigramIndex<T>`, scan và so khớp text
vào `search/`. Hai bộ chạy lần lượt, không đồng thời với build/test của dự án.

## 3. Kết quả

`Scan/index > 1` nghĩa là index nhanh hơn.

| Số phim | Query | Kết quả | Index µs | Scan µs | Scan/index |
|---:|---|---:|---:|---:|---:|
| 1.000 | `nguyen trai` | 50 | 7,273 | 44,475 | 6,12× |
| 1.000 | `nguyn trai` | 50 | 5,165 | 46,283 | 8,96× |
| 1.000 | `guy` | 50 | 2,460 | 54,744 | 22,25× |
| 1.000 | `chuong` | 982 | 89,136 | 70,824 | 0,79× |
| 1.000 | `000777` | 28 | 60,811 | 53,088 | 0,87× |
| 10.000 | `nguyen trai` | 500 | 86,614 | 914,322 | 10,56× |
| 10.000 | `nguyn trai` | 500 | 50,562 | 943,743 | 18,67× |
| 10.000 | `guy` | 500 | 27,991 | 1.195,020 | 42,69× |
| 10.000 | `chuong` | 9.982 | 2.510,655 | 2.342,172 | 0,93× |
| 10.000 | `000777` | 38 | 166,598 | 1.047,754 | 6,29× |

Đầy đủ 12 query mỗi bộ: [CSV 1.000 phim](benchmarks/film-search-1000.csv),
[CSV 10.000 phim](benchmarks/film-search-10000.csv).

| Chi phí khởi tạo, ms | 1.000 phim | 10.000 phim |
|---|---:|---:|
| Nạp toàn bộ năm TXT, gồm tạo Film và index | 22,285 | 238,091 |
| Copy Film có sẵn vào kho có index tên + ID | 7,729 | 192,486 |
| Copy cùng Film vào kho chỉ có index ID | 0,207 | 3,557 |

Hai phép copy đã có tên chuẩn hóa, **không đo lại constructor chuẩn hóa tên**.
Các phép khởi tạo đo riêng, không cộng thành tổng; không gồm giải phóng kho.
`sizeof(Film) = 80` byte trên bản build này, chưa tính vùng nhớ động của tên,
node linked list hoặc index. Chưa đo RAM/peak RSS.

## 4. Kết luận và kiểm chứng

- Index hiệu quả với query lọc được ít ứng viên. `chuong` trả gần hết phim nên
  scan nhanh hơn ở cả hai quy mô; index không thắng mọi query.
- Ít kết quả chưa chắc ít ứng viên: `000777` có trigram số phổ biến, index vẫn
  chậm hơn scan ở 1.000 phim. Cần xét posting list, không chỉ số kết quả cuối.
- Cả hai đã dùng cùng cột chuẩn hóa; lợi thế trên không đến từ việc bắt scan
  chuẩn hóa lại tên. Chưa thêm cơ chế tự chọn index/scan.
- Trigram chỉ bảo đảm các lỗi gõ còn ít nhất một trigram chung; mỗi từ tìm
  phải có ít nhất ba ký tự. Xem [thiết kế](../design/film-name-search.md).
- Release có Qt/Python: **13/13 CTest đạt**, gồm kiểm thử UI offscreen trên bản
  sao 1.000 phim, sửa tên, tìm lại, lưu/mở lại và rollback khi lỗi cấp phát.
- AddressSanitizer/UBSan và leak detection: **4/4 đạt** (`Search`, `Repository`,
  `FilmSearch`, `SampleData`); Qt chỉ kiểm tra ở Release.
- Test đối chiếu index/scan dùng chung matcher, kiểm tra đường lọc index;
  Unicode/lỗi gõ có assert riêng. Không coi dùng chung matcher là bằng chứng
  tuyệt đối cho mọi đầu vào.
