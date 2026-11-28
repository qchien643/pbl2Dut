# Xây index tên phim phục vụ tìm kiếm

Cập nhật **28/11/2026**. **Đã triển khai và kiểm thử.**
Yêu cầu và ví dụ nghiệm thu nằm ở [FR-01 trong SRS](../requirements/srs.md).
Hiện trạng giao diện nằm ở [UI-F01 trong feature map](ui-feature-map.md).

## 1. Mục đích và phạm vi

Xây **trigram inverted index** trên RAM để lọc ứng viên khi tìm tên, thay vì
duyệt mọi tên phim. Lấy ý tưởng chỉ mục đảo của GIN, không mô phỏng cấu trúc
GIN trên đĩa hoặc bộ FTS của PostgreSQL.

Chỉ đánh index tên phim. Không thêm trie, unigram/bigram, khóa nguyên từ hoặc
từ điển ngôn ngữ. Tên chuẩn hóa nằm ngay trong Film, không ở bảng phụ. Giữ nguyên
TXT, index ID, association và quy tắc nghiệp vụ hiện tại.

## 2. Cấu trúc và cách xây index

Constructor Film chuẩn hóa tên một lần vào thuộc tính chỉ đọc
`std::u32string normalizedTitle_`, lấy qua `normalizedTitle()`. Dùng UTF-32 để
so sánh theo ký tự Unicode, không phải byte UTF-8; các từ cách nhau một dấu cách.
Đây là dữ liệu dẫn xuất trên RAM, không thêm cột vào TXT.

Khi nạp/thêm phim, `FilmRepository::insert()` lưu entity, tạo index ID và index tên:

1. Đọc tên chuẩn hóa đã có: chữ thường, bỏ dấu tiếng Việt, `Đ/đ → d`.
2. Lấy đoạn ba ký tự liên tiếp, không vượt ranh giới từ.
3. Bỏ trigram trùng; thêm **con trỏ tới Film đang lưu trong kho** vào mỗi posting list.

Ví dụ tên minh họa **Nguyễn Trãi**, ID 12:

```text
Tên gốc trong Film:  Nguyễn Trãi
Tên trong cùng Film: nguyen trai  (normalizedTitle_)
Trigram:            ngu, guy, uye, yen, tra, rai

Index tên:          ngu → [Film*, ...]
                    guy → [Film*, ...]
                    tra → [Film*, ...]
Index ID đã có:     12  → địa chỉ Film trong linked list
```

Cấu trúc C++ đặt trong `FilmRepository`:

```cpp
TrigramIndex<Film> titleIndex_;
```

- `TrigramIndex<T>` ở `search/` dùng lại cho entity có `id()` và địa chỉ ổn định.
  `T` là kiểu đối tượng được trỏ tới, không xác định thuộc tính text cần tìm.
  Repository truyền nội dung thuộc tính chuẩn hóa khi ghi; mỗi cột có index riêng.
- Bên trong: `postings_` ánh xạ trigram → vector `const T*`, tăng theo ID và
  không trùng; `recordTrigrams_` ánh xạ ID → bộ trigram để sửa/xóa đúng khóa.
  Không so địa chỉ giữa các đối tượng không liên quan. Không ghi metadata vào TXT.
- Chuẩn hóa và cắt theo ký tự Unicode, **không cắt ba byte UTF-8**; hỗ trợ chữ
  Việt dựng sẵn/dấu tổ hợp. UTF-8 sai phải báo lỗi. Tên gốc luôn giữ nguyên.

Index xây cùng lúc nạp phim, giữ suốt phiên và giải phóng cùng repository.
Không ghi index vào TXT; lần mở sau xây lại từ tên đã lưu.

## 3. Dùng index để tìm

1. Chuẩn hóa câu tìm bằng cùng helper đã dùng khi xây index; tách thành các từ.
2. Với mỗi từ, tra các trigram rồi lấy **hợp** danh sách Film*. Trigram không có
   trong index không làm loại cả từ, vì câu tìm có thể bị gõ sai.
3. Lấy **giao** các tập ứng viên giữa những từ tìm: tên phải đáp ứng tất cả từ,
   không bắt buộc thứ tự. Không so vị trí các từ trong tiêu đề.
4. Đọc trực tiếp `film->normalizedTitle()` để xác nhận, **không chuẩn hóa lại
   hoặc tra ID**: mỗi từ tìm là chuỗi con của một từ trong tên, hoặc khác toàn
   bộ từ đó tối đa một thao tác thêm/xóa/thay ký tự/đảo hai ký tự liền nhau.
   Loại kết quả không đạt; tên gốc đọc ngay qua `film->title()`.

`FilmRepository::searchTitleSequential()` dùng chính `normalizedTitle()` của
từng Film trong linked list, cùng hàm xác nhận với index. Hàm này dùng để đối
chiếu trong test/benchmark; UI vẫn dùng `searchTitle()`. Khác biệt là cách lấy
ứng viên; không có bảng cache tách riêng. Hàm xác nhận dùng string_view để đọc
từ, không cấp phát chuỗi hay tách lại tên thành các bản sao.

Ví dụ câu tìm sai **`nguyn trai`**:

```text
nguyn → ngu, guy, uyn → hợp posting list của ngu/guy/uyn
trai  → tra, rai      → hợp posting list của tra/rai
                        ↓ giao hai tập Film* theo ID
Phim 12 là ứng viên → xác nhận nguyn thiếu e so với nguyen → khớp
```

Phải xác nhận bước cuối: chung trigram chưa chứng minh khớp tên. Không lấy
giao tất cả trigram của một từ vì sẽ loại ví dụ `nguyn` khi thiếu khóa `uyn`.
Không đặt ngưỡng tỷ lệ trùng tùy ý hoặc chỉ tìm lỗi gõ khi không có khớp chính xác.

### Đối chiếu năm yêu cầu

| Ví dụ tìm tên `Nguyễn Trãi` | Cách đáp ứng |
|---|---|
| `nguyen trai` | Chuẩn hóa tên và câu tìm về cùng dạng không dấu/chữ thường |
| `trai nguyen` | Giao tập Film* theo từng từ, không kiểm tra thứ tự từ |
| `nguy` | Tra `ngu`, `guy`; xác nhận tiền tố của `nguyen` |
| `guy` | Tra trigram `guy`; xác nhận chuỗi con trong `nguyen` |
| `nguyn trai` | Lấy ứng viên từ trigram chung, xác nhận lỗi thiếu một ký tự |

**Giới hạn đã thu hẹp:** chỉ cam kết các từ tìm có ít nhất ba ký tự và lỗi một
thao tác khi từ nhập/từ đích còn trigram chung. Không cam kết từ 1–2 ký tự,
lỗi mất hết trigram chung, sai nhiều ký tự hoặc lỗi trên chuỗi con. Không tự
mở rộng index để giải quyết những trường hợp ngoài yêu cầu này. Từ nhập dưới
ba ký tự báo hướng dẫn nhập thêm, không tự bỏ qua từ đó. Ô tìm trống vẫn xem
tất cả; tìm mã giữ nguyên.

## 4. Đồng bộ khi dữ liệu thay đổi

| Thao tác | Index tên |
|---|---|
| Nạp/thêm phim | Film giữ tên chuẩn hóa; tạo trigram, thêm Film* vào posting list, lưu bộ trigram theo ID |
| Sửa tên | Candidate Film có tên chuẩn hóa mới; so bộ khóa, gỡ/thêm con trỏ trên phần khác biệt, giữ địa chỉ Film |
| Chỉ sửa thời lượng hoặc tên chuẩn hóa không đổi | Không đổi index tên |
| Xóa phim | Gỡ Film* theo `recordTrigrams_` trong index trước khi giải phóng Film; xóa khóa rỗng, không gỡ phim khác |
| Thay kho tạm sau nạp | Chuyển linked list, index ID và hai bảng phụ cùng nhau |
| Lưu TXT | Chỉ lưu các cột gốc; không ghi normalizedTitle_ hoặc index |

Ví dụ đổi **Mắt Biếc → Mắt Bão**: giữ `mat`, gỡ Film* khỏi `bie/iec`, thêm vào
`bao`. Index ID và địa chỉ Film không đổi nên suất đang tham chiếu vẫn hợp lệ.

Đồng bộ nằm trong CRUD của `FilmRepository`, không nằm ở nút bấm UI. Chuẩn bị
các thay đổi có thể cấp phát trước khi commit; lỗi giữ nguyên entity/index/dirty.
Chỉ chuẩn bị các khóa bị ảnh hưởng, không copy toàn bộ kho. Xóa dựa trên bộ
trigram đã lưu, không cấp phát; thay kho phải swap cả hai bảng phụ. Không giữ
con trỏ tới value của hash table qua thao tác có thể rehash.
Posting list chuẩn bị có thể tạm giữ địa chỉ candidate; trước khi commit phải
gắn lại địa chỉ Film thật trong linked list, không lưu con trỏ tới biến tạm.
Copy/swap Film mang theo cả tên gốc và tên chuẩn hóa nên không bị lệch hai trường.

## 5. Ghép vào code và kiểm chứng

- **Helper `TextNormalization.h/.cpp`:** chỉ chuẩn hóa text về chữ thường,
  không dấu tiếng Việt; gộp dấu phân cách thành một khoảng trắng và kiểm tra
  UTF-8. Không biết phim, index hay điều kiện tìm kiếm. Film gọi helper trong
  constructor; repository gọi helper khi nhận câu tìm.
- **`FilmRepository.h/.cpp`:** sở hữu `TrigramIndex<Film>`, override CRUD để
  đồng bộ và `replaceWith()` chuyển cả index. `searchTitle()` lấy ứng viên từ
  index rồi kiểm tra đúng `normalizedTitle()`; scan chỉ dùng làm đối chứng.
- **`search/TrigramIndex.h`:** template quản lý posting, bộ trigram theo ID và
  hợp/giao ứng viên; không biết Film, tên cột hay nghiệp vụ. `write()` chuẩn bị
  index rồi gọi hàm ghi storage do repository cung cấp. Ghi lỗi thì rollback
  phần chuẩn bị; ghi thành công mới commit index bằng thao tác không cấp phát.
  Hàm ghi storage phải giữ nguyên dữ liệu khi lỗi và trả đúng địa chỉ node khi
  thành công. Đây là callback bảo toàn đồng bộ, không phải Strategy tìm kiếm.
- **`search/TextSearch.h/.cpp`:** tách/kiểm tra từ tìm đã chuẩn hóa, tạo trigram,
  xác nhận chuỗi con/lỗi một thao tác. Index và scan dùng cùng hàm `matches()`.
- **`search/SequentialSearch.h`:** duyệt range, áp dụng điều kiện rồi trả con trỏ
  chỉ đọc. Điều kiện xác định thuộc tính cần tìm; không tự chọn index.
- **Service:** CatalogService nhận một `IFilmRepository&`: CRUD kế thừa từ
  `IRepository<Film>` và `searchTitle()` nằm trong cùng hợp đồng. Không còn
  interface search riêng hay truyền kho phim hai lần. Theo quy ước thống nhất,
  ScreeningService cũng nhận `const IFilmRepository&` dù không tìm tên.
  Không thêm tìm tên vào repository của entity khác.
- **UI:** giữ luồng nhập và hiển thị; tìm theo ID, xem tất cả và các combo không
  bị thay nghĩa. Tên gốc vẫn được dùng để hiển thị.

Đã kiểm thử năm ví dụ trong SRS, Unicode, loại ứng viên sai, thêm/sửa/xóa và
khóa dùng chung, lỗi cập nhật, giữ địa chỉ Film, nạp/thay kho, lưu/mở lại, DI và
giao diện offscreen. Chi tiết và lệnh chạy ở [kiểm thử](../testing/verification.md).

Tên chuẩn hóa và index tốn thêm RAM/chi phí copy Film. Thời gian tìm phụ thuộc số posting và
ứng viên; trigram phổ biến vẫn có thể trả nhiều phim. Không tuyên bố toàn bộ
truy vấn O(1). Số đo mới 1.000/10.000 phim, cả hai cách dùng cùng cột, nằm trong
[benchmark](../testing/film-search-benchmark.md).

Nguồn: [helper](../../src/helper/TextNormalization.cpp),
[repository](../../src/repositories/FilmRepository.cpp),
[interface](../../src/repositories/IFilmRepository.h),
[test tìm kiếm](../../tests/FilmSearchTest.cpp).

## 6. Chốt cách truy xuất theo thuộc tính

| Thuộc tính/điều kiện | Cách thực hiện | Chuẩn bị |
|---|---|---|
| ID các entity | `HashTable::find()` trong `Repository<T>` | Hash index ID xây cùng CRUD |
| Tên phim | Trigram rồi xác nhận tên | Cột chuẩn hóa và `TrigramIndex<Film>` xây khi nạp |
| Ngày/phim/phòng của suất; ghế theo phòng; vé theo suất/ngày bán | Scan hiện có | Không có index phụ cho các cột này |

Không thêm enum, Strategy hay query optimizer. Các vòng lọc khác của service
chưa di chuyển trong đợt này; không thay quy tắc nghiệp vụ để tổng quát hóa scan.
Thuật toán scan dùng chung và trigram được kiểm tra thêm với hai cột text độc
lập, kiểu entity khác và điều kiện số trong [SearchTest](../../tests/SearchTest.cpp).
