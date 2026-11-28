# Kiểm thử và xác minh

Tài liệu ghi cách chạy và bằng chứng kiểm chứng các mã trong [SRS](../requirements/srs.md)
và [Business Rules Catalog](../requirements/business-rules.md); không đặt quy tắc mới.

## 1. Chạy kiểm thử

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Mong đợi 13 bộ test Passed khi có Qt và Python 3; 12 nếu `BUILD_GUI=OFF`.
Không có Python 3 thì không đăng ký `SampleGenerator`, giảm một bộ test.
Đây là số bộ thực thi CTest, không phải số ca/assert. Tests dùng assert và
chủ động bỏ `NDEBUG` để kiểm tra vẫn chạy với build Release.

## 2. Đối chiếu yêu cầu và nghiệp vụ

| Mã | Bằng chứng trong tests/ |
|---|---|
| FR-01, FR-02, BR-C01–C05 | `ServiceTest.cpp: testCatalog()`, `testCatalogNameUniqueness()`; `CinemaWindowTest` kiểm tra thao tác và lỗi nhập |
| FR-01 tìm tên | `FilmSearchTest`, `RepositoryTest` (rollback index); `SampleData` đối chiếu index/scan; `CinemaWindowTest::testLargeSampleSearch()` trên 1.000 phim |
| FR-03, BR-S01–S04 | `ServiceTest.cpp: testScheduling()`, `testCalendarAndOverflow()` |
| FR-04, BR-T01–T05 | `ServiceTest.cpp: testSales()`, `testInvalidMapsAndReconfirmation()`; `CinemaWindowTest.cpp: testCompleteWorkflow()` |
| FR-05, BR-T06 | `testSales()`, `testCompleteWorkflow()` |
| BR-D01–D05 | `ServiceTest.cpp: testDeletionPolicy()`, kiểm tra UI và round-trip trong `CinemaDatabaseTest` |
| FR-06, NFR-05 | `CinemaDatabaseTest.cpp: testEmptyAndRoundTrip()`, `testBadInput()`, `testDemoData()`; `CinemaWindowTest.cpp: testStartup()` |
| FR-07, NFR-03, NFR-04, NFR-05 | `CinemaDatabaseTest.cpp: testSaveFailures()`, `testFailedRecoveryAndValidation()`, `testRealIoFailures()`, `testAllocationFailures()`; `CinemaWindowTest.cpp: testFilmsAndClose()` |
| NFR-01 | Build ứng dụng Qt; `CinemaWindowTest` chạy offscreen, không thay thế kiểm tra màn hình thật |
| NFR-02 | `LinkedListTest`, `HashTableTest`, `Matrix2DTest`; đối chiếu kho chính trong `Repository.h` |
| NFR-03 | `ServiceTest.cpp: testAllocationRollback()`, `testLookupValidation()`, `RepositoryTest`, `EntityTest` |

Tên dạng `ServiceTest.cpp: testCatalog()` chỉ vị trí file và hàm cần đọc.

`testCatalogNameUniqueness()` bao gồm trùng khi thêm/sửa, tên sau trim, khác
hoa/thường, bỏ qua chính bản ghi, giữ ID/dirty khi lỗi và bảo toàn liên kết/giá vé.
`testScheduling()` vẫn có ca qua nửa đêm; không đổi ca đó để ngầm áp dụng giờ đóng cửa.

## 3. Kiểm tra kỹ thuật bổ sung

- `DependencyInjectionTest`: thay repository bằng implementation độc lập dùng
  `std::list`, giữ nguyên ba service; kho phim cài `IFilmRepository` bằng CRUD và scan chuỗi
  con đơn giản để chứng minh DI/DIP, không kiểm tra toàn bộ quy tắc trigram ở đây.
- `FilmSearchTest`: năm ví dụ đã chốt, chữ Việt dựng sẵn/tổ hợp, UTF-8 sai,
  CRUD/thay kho và đối chiếu scan sau nhiều thay đổi có seed cố định. Kiểm tra
  normalizedTitle_ trong Film, posting trỏ đúng node kho, thứ tự ID không theo
  thứ tự thêm và việc xóa không để lại con trỏ treo.
  Scan đối chứng gọi `searchTitleSequential()`, dùng chung `text_search::matches()` với
  `searchTitle()`; cả hai giữ nguyên kết quả đầu ra khi câu tìm không hợp lệ.
- `SearchTest`: template trigram với model khác Film, hai cột text/index độc
  lập; ghi lỗi giữ nguyên index, sửa/xóa/swap và xác nhận ứng viên sai. Scan
  dùng chung kiểm tra cột text, ID số, sức chứa phòng, dữ liệu rỗng và lỗi điều kiện.
- `RepositoryTest`: chèn lỗi từng cấp phát của thêm/sửa, kiểm tra cả entity,
  dirty và kết quả index; xóa/thay kho không cấp phát.
  `testCombinedFilmContract()` kiểm tra CRUD/tìm tên qua cùng hợp đồng, một base
  IRepository dùng chung và sửa/xóa qua interface CRUD vẫn cập nhật trigram index.
- `SampleGenerator`: đếm dữ liệu và 1.000 fixture người dùng, liên kết vé,
  sinh lại giống nhau và từ chối ghi đè; không thêm entity User vào sản phẩm.
- `ServiceTest.cpp: testQueryResultOwnership()`: kết quả truy vấn và lỗi cấp phát.
- `ServiceTest.cpp: testDateTimeHelper()`: ngày hợp lệ, năm nhuận, chuyển đổi phút.
- `ServiceTest.cpp: testSeatMapHelper()`: helper cho phép ô trống, đổi chỉ số đúng,
  từ chối kích thước sai/tràn, con trỏ rỗng và ghi đè vị trí. `testInvalidMapsAndReconfirmation()`
  kiểm tra cả CatalogService và TicketService vẫn từ chối phòng thiếu/trùng ghế.
- `CinemaDatabaseTest.cpp: testNamedRowsAndEntityFile()`, `testFileSetAndEncoding()`:
  đọc/ghi từng model, đảo cột, UTF-8 và bộ file.
- `CinemaDatabaseTest.cpp: testSchedulingRulesBelongToService()`: phân biệt kiểm tra
  lịch ở service với toàn vẹn ở database.
- `ServiceTest.cpp: testPerformanceBaseline()`: mốc thời gian trên dữ liệu test;
  không phải cam kết hiệu năng với mọi máy. Đo index tên riêng và dữ liệu lớn xem
  [benchmark](film-search-benchmark.md); bộ mẫu ở [data/sample](../../data/sample/README.md).

## 4. Kiểm tra bộ nhớ tùy chọn

```bash
cmake -S . -B build-sanitize -DBUILD_GUI=OFF \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
cmake --build build-sanitize --parallel
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir build-sanitize --output-on-failure
```

## 5. Giới hạn bằng chứng

Lệnh trên là hướng dẫn chạy, không phải xác nhận sanitizer đã chạy ở phiên hiện
tại. Test chỉ kiểm chứng các trường hợp đã viết, không chứng minh mọi đầu vào.
Thao tác thật trên năm tab theo [hướng dẫn sử dụng](../guides/user-guide.md) vẫn cần
được kiểm tra khi bàn giao. Các nhãn FR/SAT cũ còn trong comment test không phải
mã chuẩn của SRS hiện hành; dùng bảng đối chiếu tại tài liệu này.

## 6. Kiểm tra tài liệu và sơ đồ

```bash
python3 tests/check_docs.py
```

Chỉ dùng Python chuẩn, không cần Qt. Script kiểm tra đích liên kết cục bộ trong
Markdown, nguồn/liên kết trong HTML và nhãn SVG cho trình đọc màn hình;
không kiểm tra anchor `#`, ý nghĩa nghiệp vụ hay độ đúng của nội dung source.
Script cũng yêu cầu mỗi HTML nằm trong thư mục cùng tên và có `README.md`
liên kết tới nó. `slides/` và `ref/` không thuộc phạm vi kiểm tra.

Đợt tài liệu ngày 27/11/2026: bảy hình đã qua `self_check.py` và
`verify_geometry.py` của Diagram Design, xem ảnh render trên Chromium; kiểm tra
cuộn ngang ở viewport 390px. Ảnh năm tab được chụp từ Qt Fusion/offscreen trên
bản sao dữ liệu demo, không sửa dữ liệu gốc. Đây không phải kiểm chứng toàn bộ
thao tác bằng chuột trên desktop thật. Build và CTest: 9/9 bộ test Passed.

Đợt index tên ngày 28/11/2026: build Release Qt/Python, **12/12 bộ Passed**;
`CinemaWindowTest` dùng bản sao bộ 1.000 phim để tìm, đổi tên, lưu và nạp lại.
Không coi số này là xác nhận đã vẽ lại cả bảy sơ đồ hoặc chụp lại ảnh hướng dẫn.

Cùng đợt đã build Debug `BUILD_GUI=OFF` với
`-fsanitize=address,undefined -fno-omit-frame-pointer`, chạy `Repository`,
`FilmSearch`, `SampleData` với `ASAN_OPTIONS=detect_leaks=1`: **3/3 Passed**,
không có báo lỗi sanitizer trong ba bộ này. Không suy ra toàn bộ Qt đã chạy
dưới sanitizer.

Đợt tách thuật toán dùng chung cùng ngày: Release Qt/Python **13/13 Passed**;
Debug sanitizer chạy `Search`, `FilmSearch`, `Repository`, `SampleData`:
**4/4 Passed**, không có báo lỗi sanitizer. Hai phương pháp tiếp tục trả cùng
tập kết quả trên bộ 1.000 và 10.000 phim; số đo ở [benchmark](film-search-benchmark.md).

Đợt gộp hợp đồng kho phim: Release **13/13 Passed**; sanitizer chạy lại
`Repository`, `Service`, `DependencyInjection`, `CinemaDatabase`, `FilmSearch`,
`Search`, `SampleData`: **7/7 Passed**, không có báo lỗi sanitizer. Không có
`IFilmSearch` riêng; CatalogService dùng cùng `IFilmRepository` cho CRUD/tìm tên.
