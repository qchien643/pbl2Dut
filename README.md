# PBL2 - Cinema Management System

Đồ án PBL2 quản lý rạp chiếu phim chạy offline trên một máy, viết bằng C++17 và
Qt 6 Widgets. Dữ liệu được lưu trong năm file TXT, không dùng DBMS hoặc kết nối
mạng. Phiên bản hiện tại đã hoàn thiện giao diện, nghiệp vụ và kiểm thử.

## Chức năng

- Quản lý phim, phòng chiếu và sơ đồ ghế.
- Tạo, sửa, xóa và lọc suất chiếu; kiểm tra trùng lịch và thời gian nghỉ.
- Chọn ghế, xác nhận tiền mặt và bán vé.
- Xem toàn bộ vé bán theo ngày, tìm nhanh theo mã và tính doanh thu ngày.
- Kiểm tra toàn vẹn dữ liệu khi mở; lưu đồng bộ năm file và giữ bản sao phục hồi.

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

Kết quả đúng là toàn bộ 8 test đều `Passed`. CMake cũng tạo
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
cp -r data/demo /tmp/pbl2-demo
./build/cinema /tmp/pbl2-demo
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
├── DataStructures/   LinkedList, HashTable, Matrix2D tự cài đặt
├── entities/         Mỗi entity có cặp .h/.cpp riêng
├── repositories/     Mỗi repository có một header riêng
├── persistence/      Đọc và ghi định dạng TXT
├── services/         Mỗi service có header và implementation riêng
├── ui/               Giao diện Qt Widgets
├── ApplicationData.* Điều phối nạp, kiểm tra và lưu dữ liệu
└── main.cpp           Điểm khởi động chương trình
tests/                 Tám bộ kiểm thử tự chạy bằng CTest
data/demo/             Bộ dữ liệu mẫu
docs/design/           Thiết kế dữ liệu, lớp, TXT và giao diện
docs/requirements/     Đặc tả yêu cầu hiện hành
```

Quy tắc nghiệp vụ nằm trong service, không nằm trong widget. Repository dùng cấu
trúc dữ liệu tự cài đặt làm nơi lưu chính; Qt chỉ phục vụ giao diện.

Cách trình bày tham khảo `docs/ref/oop`: lớp thường tách `.h/.cpp`, template khai
báo trước và định nghĩa bên dưới trong header; dùng kiểu tường minh và vòng lặp
thay callback duyệt dữ liệu. Giữ `Repository<T>` dùng chung. Các ngoại lệ vì an
toàn bộ nhớ và kết nối sự kiện Qt được giải thích trong tài liệu thiết kế lớp.

## Kiến trúc

```text
MainWindow (Qt UI)
        │
        ▼
CatalogService / ScreeningService / TicketService
        │
        ▼
Repository<T> ── LinkedList<T> + HashTable<ID, T*>
        ▲
        │
ApplicationData ── TxtPersistence ── 5 file TXT
```

- Entity đóng gói dữ liệu và kiểm tra tính hợp lệ nội tại.
- Service xử lý các quy tắc cần phối hợp nhiều entity.
- Repository quản lý bản ghi và chỉ mục trên RAM.
- `ApplicationData` điều phối nạp, kiểm tra, lưu và phục hồi dữ liệu.
- `Matrix2D<T>` được dùng để dựng sơ đồ ghế của phòng hoặc suất chiếu.

## Tài liệu

- [Mục lục tài liệu OOP và database](docs/README.md)
- [Lộ trình đọc code và vấn đáp 1:1](docs/study-guide.md)
- [Đặc tả yêu cầu](docs/requirements/srs.md)
- [Thiết kế dữ liệu và ERD](docs/design/data-design.md)
- [Thiết kế lớp và tổ chức RAM](docs/design/class-design.md)
- [Sơ đồ Archify: luồng hệ thống, class, kiến trúc và ERD](docs/diagrams/README.md)
- [Định dạng TXT và phục hồi](docs/design/txt-format.md)
- [Đối chiếu triển khai và kiểm thử](docs/design/verification.md)
- [Wireframe giao diện](docs/design/ui-wireframe.html)

## Xử lý lỗi thường gặp

`Could not find Qt6`:

```bash
sudo apt install qt6-base-dev
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

VS Code gạch đỏ các dòng `#include <Q...>`: build dự án một lần, sau đó chạy
`C/C++: Reset IntelliSense Database` hoặc `Developer: Reload Window`.
