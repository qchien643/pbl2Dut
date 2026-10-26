# Tài liệu đồ án OOP và thiết kế dữ liệu

Bản đối chiếu mã nguồn ngày 26/10/2026. Sản phẩm là ứng dụng C++17/Qt,
lưu TXT; không phải ứng dụng đã tích hợp hệ quản trị cơ sở dữ liệu.

## Đọc theo mục tiêu

| Cần làm gì? | Tài liệu |
|---|---|
| Cài, build và chạy trên máy khác | [README dự án](../README.md) |
| Nắm bài toán và tiêu chí nghiệm thu | [SRS](requirements/srs.md) |
| Giải thích ERD, khóa, phụ thuộc hàm, 3NF và toàn vẹn | [Thiết kế dữ liệu](design/data-design.md) |
| Giải thích đóng gói, kế thừa, composition, template, bộ nhớ | [Thiết kế lớp](design/class-design.md) |
| Xem sơ đồ tương tác kiến trúc, lớp và ERD | [Sơ đồ Archify](diagrams/README.md) |
| Hiểu parser, lưu nhiều file và phục hồi | [Định dạng TXT](design/txt-format.md) |
| Học code, luyện câu hỏi và thực hành sửa nhỏ | [Hướng dẫn vấn đáp](study-guide.md) |
| Tìm bằng chứng kiểm thử và giới hạn kiểm chứng | [Đối chiếu kiểm thử](design/verification.md) |

SRS ghi **cần làm gì**; thiết kế giải thích **đang làm bằng cách nào và vì sao**;
hướng dẫn vấn đáp giúp **tự lần theo code**. Không coi kết quả test là chứng minh
chương trình đúng trong mọi trường hợp.

## Cách sử dụng khi ôn tập

1. Đọc SRS và vẽ lại năm thực thể, khóa và quan hệ mà không nhìn tài liệu.
2. Đọc thiết kế lớp, chỉ đúng khai báo thể hiện mỗi quan hệ trong `src`.
3. Theo thứ tự file trong hướng dẫn vấn đáp; tự giải thích rồi mới xem đáp án.
4. Lần theo một vé từ nút bấm đến RAM, TXT, mở lại và thống kê.
5. Chạy các test tương ứng và thử sửa một yêu cầu nhỏ trên bản sao dự án.

Tham khảo cách giảng trong [ObjectRelation.pdf](slides/ObjectRelation.pdf),
[Template.pdf](slides/Template.pdf) và [code môn học](ref/oop).
Các tài liệu này được giữ nguyên. Không suy luận quyền sở hữu chỉ từ tên quan hệ
hoặc sao chép cách quản lý con trỏ mà chưa kiểm tra constructor/destructor.

[Wireframe](design/ui-wireframe.html) chỉ là minh họa giao diện; mã Qt và hành vi
kiểm thử mới là căn cứ mô tả sản phẩm hiện tại.
