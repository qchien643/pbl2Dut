# PBL2 - Cinema Management System

Đồ án PBL2 quản lý rạp chiếu phim chạy offline trên một máy, viết bằng C++17 và
Qt 6 Widgets. Dữ liệu được lưu trong năm file TXT, không dùng DBMS hoặc kết nối
mạng. Hướng dẫn dưới đây dành cho cài đặt và chạy dự án.

## Chức năng

- Quản lý phim, phòng chiếu và sơ đồ ghế.
- Tạo, sửa, xóa và lọc suất chiếu; kiểm tra trùng lịch và thời gian nghỉ.
- Chọn ghế, xác nhận tiền mặt và bán vé.
- Xem toàn bộ vé bán theo ngày, tìm nhanh theo mã và tính doanh thu ngày.
- Kiểm tra toàn vẹn dữ liệu khi mở; lưu đồng bộ năm file và giữ bản sao phục hồi.

Cách vận hành xem [hướng dẫn sử dụng](docs/guides/user-guide.md); điều kiện cho phép/cấm
xem [quy tắc nghiệp vụ](docs/requirements/business-rules.md).

## Cài đặt công cụ

### Ubuntu/Debian

Máy cần Git, compiler C++17, CMake và Qt 6 Widgets:

```bash
sudo apt update
sudo apt install -y git build-essential cmake qt6-base-dev
```

### Windows 10/11

Cài [MSYS2](https://www.msys2.org/), mở terminal **MSYS2 UCRT64** rồi chạy:

```bash
pacman -Syu
pacman -S --needed git mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja \
    mingw-w64-ucrt-x86_64-qt6-base
```

Các lệnh còn lại phải chạy trong terminal UCRT64, không chạy bằng Command Prompt.

## Clone dự án

Clone và vào thư mục dự án:

```bash
git clone https://github.com/qchien643/pbl2Dut.git
cd pbl2Dut
```

## Build và kiểm thử

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Trên MSYS2 UCRT64, thêm Ninja ở bước cấu hình:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

Kết quả đúng là 12 test đều `Passed` khi có Qt/Python 3 (11 nếu không build Qt;
thiếu Python thì bớt test sinh dữ liệu). CMake cũng tạo
`build/compile_commands.json` để VS Code nhận các header Qt sau lần build đầu tiên.

Nếu chỉ cần kiểm thử phần lõi mà không có Qt:

```bash
cmake -S . -B build -DBUILD_GUI=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Chạy chương trình

Không truyền tham số, chương trình dùng trực tiếp bộ dữ liệu mẫu `data/demo`:

```bash
./build/cinema
```

Để thử mà không sửa bộ dữ liệu mẫu, truyền đường dẫn tới một bản sao:

```bash
demo_copy=$(mktemp -d /tmp/pbl2-demo.XXXXXX)
cp data/demo/*.txt "$demo_copy/"
./build/cinema "$demo_copy"
```

Thư mục dữ liệu phải tồn tại và chứa đủ:

```text
films.txt
auditoriums.txt
seats.txt
screenings.txt
tickets.txt
```

Có thể chọn thư mục rỗng để bắt đầu dữ liệu mới sau khi xác nhận. Khi thoát bình
thường, chương trình lưu toàn bộ dữ liệu; các thư mục `.cms-backup-N` là bản sao
phục hồi và không nên xóa khi chưa kiểm tra dữ liệu hiện tại.

## Cấu trúc dự án

```text
src/
├── DataStructures/    LinkedList, HashTable, Matrix2D tự cài đặt
├── entities/          Mỗi entity có cặp .h/.cpp riêng
├── repositories/      Mỗi repository có một header riêng
├── fileio/            TextFile đọc TXT; EntityFile chuyển đổi entity
├── helper/            Ngày giờ, kiểm tra trường dữ liệu và dựng ma trận ghế
├── services/          Mỗi service có header và implementation riêng
├── gui/               CinemaWindow: cửa sổ Qt Widgets
├── database/          CinemaDatabase: sở hữu repository, nạp và lưu dữ liệu
└── main.cpp           Điểm khởi động chương trình
tests/                 Các bộ CTest và script kiểm tra tài liệu/dữ liệu mẫu
data/demo/             Bộ dữ liệu mẫu
data/sample/           1.000 phim, 20 phòng, 2.000 ghế/suất và 5.000 vé
tools/                 Sinh dữ liệu lớn và benchmark tìm tên
docs/design/           Thiết kế dữ liệu, lớp, service và TXT
docs/requirements/     Yêu cầu sản phẩm và danh mục quy tắc nghiệp vụ
docs/diagrams/         Bảy sơ đồ; mỗi hình có README.md giải thích
docs/guides/           Hướng dẫn sử dụng, ảnh Qt và lộ trình vấn đáp
docs/testing/          Đối chiếu yêu cầu, nghiệp vụ và kiểm thử
```

Kiến trúc và ownership xem [thiết kế lớp](docs/design/class-design.md);
luồng từng hàm xem [thiết kế service](docs/design/service-design.md).

## Tài liệu

- [Mục lục tài liệu OOP và database](docs/README.md)
- [Lộ trình đọc code và vấn đáp 1:1](docs/guides/study-guide.md)
- [Hướng dẫn sử dụng năm tab](docs/guides/user-guide.md)
- [Feature map từng tab: tìm kiếm, bộ lọc và nội dung thảo luận](docs/design/ui-feature-map.md)
- [Bảy sơ đồ: kiến trúc, UML, DI/DIP, ERD và luồng xử lý](docs/diagrams/README.md)
- [Đối chiếu triển khai và kiểm thử](docs/testing/verification.md)
- [Bộ dữ liệu lớn và cách chạy](data/sample/README.md)
- [Thiết kế index tên phim](docs/design/film-name-search.md)
- [Benchmark index so với tra tên tuần tự](docs/testing/film-search-benchmark.md)

## Xử lý lỗi thường gặp

`Could not find Qt6`:

```bash
sudo apt install qt6-base-dev
cmake -S . -B build -DBUILD_GUI=ON -DCMAKE_BUILD_TYPE=Release
```

VS Code gạch đỏ các dòng `#include <Q...>`: build dự án một lần, sau đó chạy
`C/C++: Reset IntelliSense Database` hoặc `Developer: Reload Window`.
