# Kiểm thử và xác minh

## 1. Chạy kiểm thử

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Kết quả mong đợi: **8/8 test Passed**.

## 2. Các bộ test

| Test | Nội dung |
|---|---|
| `LinkedListTest` | Thêm, duyệt, xóa và quản lý node |
| `HashTableTest` | Va chạm, rehash, insert/find/erase |
| `Matrix2DTest` | Kích thước, truy cập biên và ánh xạ ô |
| `EntityTest` | Constructor, getter và direct association |
| `RepositoryTest` | ID, index, dirty và API `TicketRepository` |
| `ServiceTest` | CRUD, lịch chiếu, bán vé, doanh thu và rollback |
| `ApplicationDataTest` | Parser, toàn vẹn, lưu và phục hồi file |
| `MainWindowTest` | Luồng Qt từ thao tác người dùng đến service |

## 3. Đối chiếu yêu cầu

| Yêu cầu | Bằng chứng chính |
|---|---|
| CRUD phim/phòng/suất | `ServiceTest`, `MainWindowTest` |
| Trùng lịch và thời gian nghỉ | `ServiceTest` |
| Bán trùng ghế, giá chốt | `ServiceTest`, `EntityTest` |
| Vé và doanh thu theo ngày | `ServiceTest`, `MainWindowTest` |
| Nạp dữ liệu sai | `ApplicationDataTest` |
| Lưu lỗi và phục hồi | `ApplicationDataTest` |
| Cấu trúc dữ liệu tự cài | Ba test DSA |

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
