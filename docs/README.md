# Tài liệu đồ án PBL2

Cập nhật ngày **27/10/2026**. Dự án là ứng dụng C++17/Qt chạy offline, lưu dữ
liệu bằng năm file TXT. Phần database được thể hiện qua mô hình quan hệ, khóa và
ràng buộc; dự án không dùng DBMS hay SQL.

## Mục lục

| Tài liệu | Nội dung chính |
|---|---|
| [README dự án](../README.md) | Cài đặt, build, chạy và cấu trúc thư mục |
| [SRS](requirements/srs.md) | Phạm vi và yêu cầu sản phẩm |
| [Thiết kế dữ liệu](design/data-design.md) | ERD, khóa, ràng buộc và chuẩn hóa |
| [Thiết kế lớp](design/class-design.md) | OOP, direct association, repository và service |
| [Định dạng TXT](design/txt-format.md) | Nạp, lưu và phục hồi dữ liệu |
| [Kiểm thử](design/verification.md) | Lệnh chạy test và phạm vi kiểm chứng |
| [Sơ đồ Archify](diagrams/README.md) | Luồng hệ thống, class, kiến trúc và ERD |
| [Hướng dẫn vấn đáp](study-guide.md) | Lộ trình đọc code và câu hỏi ôn tập |

## Thứ tự nên đọc

1. SRS để hiểu bài toán.
2. Thiết kế dữ liệu và sơ đồ ERD.
3. Thiết kế lớp và sơ đồ class.
4. Lần theo ba service trong hướng dẫn vấn đáp.
5. Đọc vòng đời TXT rồi chạy test.

Tài liệu trong `slides/` và `ref/oop/` là tài liệu tham khảo môn học, không phải
đặc tả của sản phẩm hiện tại.
