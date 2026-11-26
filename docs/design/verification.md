# Kiểm thử và xác minh

## 1. Chạy kiểm thử

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Kết quả mong đợi: **9/9 test Passed** (8 test nếu `BUILD_GUI=OFF`).

## 2. Các bộ test

| Test | Nội dung |
|---|---|
| `LinkedListTest` | Thêm, duyệt, xóa và quản lý node |
| `HashTableTest` | Va chạm, rehash, insert/find/erase |
| `Matrix2DTest` | Kích thước, truy cập biên và ánh xạ ô |
| `EntityTest` | Constructor, getter và direct association |
| `RepositoryTest` | ID, index, dirty và CRUD vé qua `IRepository<Ticket>`; update giữ địa chỉ |
| `ServiceTest` | CRUD, lịch chiếu, bán vé, doanh thu và rollback |
| `DependencyInjectionTest` | Chạy ba service với cùng template repository độc lập bằng `std::list` cho cả năm entity; bảo toàn association và chặn bán trùng |
| `ApplicationDataTest` | Đọc Row và chuyển đổi năm model, đảo cột, tham chiếu, toàn vẹn, lỗi ghi/đóng file và phục hồi |
| `MainWindowTest` | Luồng Qt từ thao tác người dùng đến service |

## 3. Đối chiếu yêu cầu

| Yêu cầu | Bằng chứng chính |
|---|---|
| CRUD phim/phòng/suất | `ServiceTest`, `MainWindowTest` |
| Quy ước xóa, cascade ghế và bảo toàn con trỏ vé | `testDeletionPolicy()` trong `ServiceTest`, `MainWindowTest`, `ApplicationDataTest` |
| Trùng lịch và thời gian nghỉ | `ServiceTest` |
| Bán trùng ghế, giá chốt | `ServiceTest`, `EntityTest` |
| Vé và doanh thu theo ngày | `ServiceTest`, `MainWindowTest` |
| Nạp dữ liệu sai | `ApplicationDataTest` |
| Lưu lỗi và phục hồi | `ApplicationDataTest` |
| Cấu trúc dữ liệu tự cài | Ba test DSA |
| DI/DIP tại ranh giới service–repository | `DependencyInjectionTest` |

## 4. Kiểm tra bộ nhớ

Có thể build phần lõi với AddressSanitizer và UndefinedBehaviorSanitizer:

```bash
cmake -S . -B build-sanitize -DBUILD_GUI=OFF \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
cmake --build build-sanitize --parallel
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir build-sanitize --output-on-failure
```

## 5. Giới hạn của test

Test chỉ chứng minh các trường hợp đã viết, không chứng minh chương trình đúng
với mọi đầu vào hoặc mọi lỗi hệ thống. Khi vấn đáp cần đọc cả điều kiện chuẩn bị,
thao tác và `assert`, không chỉ nêu tên test.
