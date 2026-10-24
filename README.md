# Cinema Data Core

Lõi dữ liệu C++17 cho bài tập quản lý rạp chiếu phim và bán vé POS. Thiết kế ban đầu gồm entity, repository, quy tắc nghiệp vụ, lưu TXT và kiểm thử. Theo yêu cầu dọn dự án, workspace hiện chỉ giữ lại ba header cấu trúc dữ liệu; mã nghiệp vụ, cấu hình build và test không còn trong thư mục.

## Cấu trúc hiện được giữ lại

- [DoubleLinkedList.h](./include/cinema/structer/DoubleLinkedList.h): danh sách liên kết đôi tự quản lý node, hỗ trợ thêm, chèn có thứ tự, tìm, xóa, duyệt và copy/move.
- [HashTable.h](./include/cinema/structer/HashTable.h): bảng băm xử lý va chạm bằng chaining với `DoubleLinkedList`.
- [Matrix2D.h](./include/cinema/structer/Matrix2D.h): ma trận hai chiều cấp phát động, truy cập có kiểm tra biên.

Các header không dùng container STL để lưu dữ liệu. `HashTable` nhận hàm băm và hàm so sánh qua template; kiểu khóa cần chính sách băm phù hợp.

## Tham khảo thiết kế ban đầu

Các tài liệu trong thư mục này mô tả phạm vi dự án trước khi được rút gọn:

- [idea.md](./idea.md): mục tiêu và phạm vi nghiệp vụ.
- [structure.md](./structure.md): entity, chỉ mục và bất biến dữ liệu.
- [prompt.md](./prompt.md): yêu cầu triển khai ban đầu.
- [Remad.md](./Remad.md): ghi chú ôn tập cấu trúc dữ liệu và OOP.
- [graphics.md](./graphics.md) và [graphics.html](./graphics.html): phác thảo giao diện POS.
- [docs/design/txt-format.md](./docs/design/txt-format.md): định dạng lưu dữ liệu TXT.

Một số tài liệu đã được dựng lại từ mô tả còn lưu trong phiên làm việc, không phải bản sao lưu nguyên văn. Cấu hình CMake và Qt 6 trước đây đã bị xóa theo yêu cầu chỉ giữ cấu trúc dữ liệu; vì vậy hiện chưa có lệnh build/test toàn dự án.
