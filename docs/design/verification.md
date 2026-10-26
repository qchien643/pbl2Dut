# ĐỐI CHIẾU TRIỂN KHAI VÀ KIỂM THỬ

**Cập nhật:** 26/10/2026. Dùng để tìm bằng chứng cho yêu cầu và thiết kế.

## 1. Ma trận yêu cầu

| Phạm vi | Triển khai | Bằng chứng kiểm thử chính |
|---|---|---|
| `FR-FILM-*` | `CatalogService`, tab `Phim` | ServiceTest: `testCatalog`; MainWindowTest: `testFilmsAndClose` |
| `FR-ROOM-*` | `CatalogService`, tab `Phòng & ghế` | ServiceTest: `testCatalog`, `testAllocationRollback`; MainWindowTest: `testCompleteWorkflow` |
| `FR-SHOW-*` | `ScreeningService`, tab `Suất chiếu` | ServiceTest: `testScheduling`, `testCalendarAndOverflow`; MainWindowTest: `testCompleteWorkflow` |
| `FR-SEARCH-*`, `FR-SALE-*` | `TicketService`, tab `Bán vé` | ServiceTest: `testSales`, `testInvalidMapsAndReconfirmation`; MainWindowTest: `testCompleteWorkflow` |
| `FR-QUERY-*`, `FR-REPORT-*` | `TicketService`, tab `Vé và doanh thu` | ServiceTest: `testSales`; MainWindowTest: `testCompleteWorkflow` |
| `FR-DATA-*` | `TxtPersistence`, `ApplicationData`, `MainWindow::closeEvent` | ApplicationDataTest; MainWindowTest: `testStartup`, `testFilmsAndClose` |
| `NFR-02` | Duyệt DSA trên RAM | ServiceTest: `testPerformanceBaseline`, tải SRS và ngưỡng 2 giây cho các truy vấn được đo |
| `NFR-03`, `NFR-11` | Rollback, kiểm tra tràn, RAII | RepositoryTest; ServiceTest: `testAllocationRollback`; các test lỗi cấp phát/lifetime |
| `NFR-04`, `NFR-07` | Lưu đủ năm file, phục hồi lỗi, mở lại | ApplicationDataTest; MainWindowTest: `testCompleteWorkflow` |

Bảng chỉ vị trí kiểm thử liên quan, không thay việc đọc từng assert hoặc chứng
minh mọi đường chạy đều được bao phủ. Các ID SAT trong SRS là kịch bản nghiệm thu;
mỗi kịch bản có thể cần đối chiếu nhiều hàm test.
Tên như ServiceTest chỉ file `tests/ServiceTest.cpp`, không phải một lớp hay namespace.

## 2. Tám chương trình kiểm thử CTest

| CTest | Nội dung |
|---|---|
| `LinkedList` | Thêm, tìm, xóa, lifetime, exception safety và tái sử dụng template. |
| `HashTable` | Va chạm, khóa chuỗi, giá trị con trỏ, lỗi cấp phát và chỉ mục repository. |
| `Matrix2D` | Biên chỉ số, kích thước sai, lifetime và ánh xạ ghế hai chiều. |
| `Entity` | Dữ liệu hợp lệ/sai, chuẩn hóa và ngày giờ. |
| `Repository` | CRUD, ID, năm repository cụ thể và rollback khi cập nhật/cấp phát lỗi. |
| `Service` | Danh mục, xếp lịch, bán vé, tra cứu, doanh thu, rollback và hiệu năng. |
| `ApplicationData` | TXT, UTF-8, round-trip, bộ file, lỗi I/O, phục hồi và dữ liệu demo. |
| `MainWindow` | Khởi động, biểu mẫu, năm tab, luồng bán vé, danh sách vé theo ngày và lưu khi đóng. |

Các hàm kiểm thử trọng tâm:

- `ServiceTest`: `testCatalog`, `testScheduling`, `testCalendarAndOverflow`,
  `testSales`, `testAllocationRollback`, `testInvalidMapsAndReconfirmation`,
  `testPerformanceBaseline`.
- `ApplicationDataTest`: `testEmptyAndRoundTrip`, `testBadInput`,
  `testFileSetAndEncoding`, `testSaveFailures`, `testFailedRecoveryAndValidation`,
  `testRealIoFailures`, `testAllocationFailures`, `testDemoData`.
- `MainWindowTest`: `testStartup`, `testFilmsAndClose`, `testCompleteWorkflow`.

`data/demo/` được nạp thật trong `testDemoData` của ApplicationDataTest; không phải dữ
liệu được mã hóa cứng trong chương trình. Test Qt chạy bằng platform `offscreen`.

## 3. Lệnh xác minh bản bàn giao

Build đầy đủ Qt 6 ở ngoài thư mục nguồn:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Xác minh ngày 26/10/2026: bản Release có Qt đạt **8/8**; bản Debug không Qt
với UndefinedBehaviorSanitizer đạt **7/7**, không ghi nhận lỗi từ sanitizer.
`LinkedListTest` kiểm tra vòng lặp sửa/đọc và xóa đúng địa chỉ;
`RepositoryTest` giữ kiểm tra chỉ được duyệt đọc. `ApplicationDataTest`
kiểm tra thêm số dòng báo lỗi ở ghế, phòng, suất và vé sau khi bỏ callback.

Nếu máy chỉ có toolchain C++ và chưa cài Qt, có thể kiểm tra bảy bộ test phần lõi:

```bash
cmake -S . -B build -DBUILD_GUI=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Hướng dẫn cài Qt, build và chạy trên Ubuntu/Debian và Windows nằm trong `README.md`.

## 4. Giới hạn của kết quả

- Assert xác nhận dữ liệu đầu ra và trạng thái trong những trường hợp được chạy,
  không chứng minh đúng với mọi đầu vào.
- UBSan hỗ trợ phát hiện một số hành vi không xác định; không phải công cụ
  chứng minh không rò bộ nhớ, không thay kiểm tra ownership.
- Test Qt offscreen kiểm tra hành vi widget, không thay thử giao diện thật,
  khả năng đọc hoặc thao tác của người dùng.
- Test lỗi I/O dùng cả tình huống thực và lỗi giả lập. Không phải thử nghiệm
  mất điện vật lý hay bảo đảm transaction ACID.
- Ngưỡng hiệu năng hai giây áp dụng các truy vấn trong `testPerformanceBaseline`
  và môi trường chạy test; không phải mọi thao tác đều O(1), cũng không đo toàn
  bộ thời gian nạp/validate/lưu.
- Kết quả Linux tại đây không xác nhận đã kiểm thử trên mọi phiên bản Windows.

## 5. Chứng minh hiểu code khi vấn đáp

| Thiết kế cần bảo vệ | Bằng chứng nên mở |
|---|---|
| Repository chỉ cho đọc trực tiếp và không copy | `static_assert` ở cuối `tests/RepositoryTest.cpp` |
| Lỗi insert/update không làm hỏng bản ghi | `testInsertionRollback`, `testUpdateRollback` trong RepositoryTest |
| List xóa đúng địa chỉ, kể cả giá trị trùng | `testReadOnlyIterationAndErase` trong LinkedListTest |
| Rehash giữ dữ liệu khi copy/hash lỗi | `testFailures` trong HashTableTest |
| Sửa phòng hoặc bán vé không đổi dữ liệu một phần | `testAllocationRollback` trong ServiceTest |
| Parser không chấp nhận dữ liệu hỏng | `testBadInput`, `testFileSetAndEncoding` trong ApplicationDataTest |
| Lưu lỗi giữ RAM và hỗ trợ thử lại | `testSaveFailures`, `testFailedRecoveryAndValidation` trong ApplicationDataTest |
| Bán → tra cứu ngày → lưu → mở lại | `testCompleteWorkflow` trong MainWindowTest |

Với mỗi test, tự chỉ ra phần chuẩn bị dữ liệu, thao tác đang thử và assert chứng
minh điều gì. Không coi tên test là bằng chứng nếu chưa đọc điều kiện bên trong.
