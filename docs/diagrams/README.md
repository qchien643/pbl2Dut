# Sơ đồ Archify

Mở trực tiếp các file HTML bằng trình duyệt; không cần chạy ứng dụng Qt.

## Sơ đồ tham khảo

| Sơ đồ | Nội dung |
|---|---|
| [Luồng xử lý hệ thống](workflow/index.html) | Khởi động, nạp TXT, thao tác Qt, service, repository, lưu và lỗi |
| [Class, Service và Repository](classes/index.html) | Direct association, kế thừa `Repository<T>`, composition và service |

Hai sơ đồ trên được đối chiếu với commit `3395a7e` trên `main`. Mỗi node có thể
mở phần nguồn tương ứng. Bản HTML đã qua validate, artifact check và browser check.
Chúng chưa phản ánh DI/DIP, ánh xạ TXT và việc thống nhất năm repository qua
`IRepository<T>` ngày 26/11/2026 (vé không còn dùng composition);
đọc [thiết kế lớp](../design/class-design.md) và [định dạng TXT](../design/txt-format.md)
để đối chiếu code hiện tại.

## Sơ đồ bổ sung

| Sơ đồ | Nội dung |
|---|---|
| [Kiến trúc tổng quát](architecture/index.html) | Phân tầng Qt, service, RAM và TXT |
| [ERD](erd/index.html) | Năm thực thể, PK/FK và lực lượng quan hệ |

Sơ đồ bổ sung được tạo từ snapshot cũ hơn; dùng để tham khảo bố cục. Thiết kế
direct association hiện tại phải đọc theo
[thiết kế lớp](../design/class-design.md).

Nội dung sơ đồ bằng tiếng Việt; thanh công cụ Archify dùng tiếng Anh. Các file
`candidate.json` là nguồn để tạo lại HTML; Archify chỉ là công cụ viết tài liệu,
không phải dependency của chương trình C++.
