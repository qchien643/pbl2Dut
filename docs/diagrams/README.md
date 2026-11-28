# Sơ đồ dự án

Bảy sơ đồ dưới đây được đối chiếu source ngày **27/11/2026**.
Mỗi HTML mở trực tiếp bằng trình duyệt, không cần Qt hay server;
Google Fonts có font hệ thống dự phòng. Khổ rộng, bản tĩnh, có cuộn ngang trên
màn hình nhỏ và hỗ trợ in.

## 1. Bộ sơ đồ hiện hành

Mỗi liên kết dưới đây mở bản mô tả Markdown; chọn **Mở sơ đồ HTML** ở đầu trang
để xem hình. Mỗi thư mục chỉ chứa một `README.md` và HTML cùng tên thư mục.
Khi xem trên GitHub, đọc Markdown trực tiếp; tải/mở HTML bằng trình duyệt để
xem hình, vì GitHub không chạy HTML trong trang xem source.

| Sơ đồ | Câu hỏi được giải đáp | Tài liệu chịu trách nhiệm |
|---|---|---|
| [Kiến trúc hệ thống](system-architecture/README.md) | Qt, service, database, kho RAM và file gọi nhau thế nào? | [Thiết kế lớp](../design/class-design.md) |
| [UML năm entity](entities-uml/README.md) | Thuộc tính nào thể hiện association; ai trỏ tới ai? | [Thiết kế lớp](../design/class-design.md#2-quan-hệ-giữa-entity) |
| [UML repository–service và DI/DIP](repository-service-uml/README.md) | Interface, kế thừa, thành phần sở hữu và injection ở đâu? | [Thiết kế lớp](../design/class-design.md#5-service) |
| [ERD năm bảng](data-erd/README.md) | PK/FK và bội số khác con trỏ RAM thế nào? | [Thiết kế dữ liệu](../design/data-design.md) |
| [Trình tự bán một vé](ticket-sale-sequence/README.md) | Từ UI đến sell, kiểm tra lại, insert và trả mã vé ra sao? | [Thiết kế service](../design/service-design.md#3-ticketservice) |
| [Sửa/xóa phòng](room-change-flow/README.md) | Khi nào chặn; vì sao sửa cần relink nhưng xóa không cần? | [Thiết kế service](../design/service-design.md#1-catalogservice) |
| [Vòng đời TXT → RAM → TXT](txt-lifecycle/README.md) | Ba hình chi tiết: nạp bộ dữ liệu, theo dõi một dòng ghế và lưu/thoát; kèm sở hữu đối tượng trên RAM và các nhánh lỗi. | [Thiết kế TXT](../design/txt-format.md) |

Các hình ghi rõ phần gộp/bỏ và liên kết về source ở cuối trang; không phải bản
liệt kê toàn bộ class/hàm. Sơ đồ kiến trúc dùng mũi tên hướng gọi; UML dùng ký
hiệu OOP; ERD dùng khóa và bội số. Không suy ra composition giữa entity từ
chính sách xóa dữ liệu.

Khi sửa source, cập nhật hình thuộc chủ đề đó và chạy
`python3 tests/check_docs.py`. Kiểm tra liên kết không thay thế review ý nghĩa
mũi tên, nghiệp vụ hoặc hình hiển thị.

## 2. Quy tắc bảo trì

Không thêm bản sao HTML/JSON từ công cụ vào một thư mục riêng để làm thiết kế
thứ hai. HTML hiện tại là file tự chứa; Markdown giải thích cách đọc và chỉ
đường tới tài liệu chi tiết, không chép lại toàn bộ BR hoặc header C++.
Khi nguồn thay đổi, đối chiếu nội dung rồi sửa cặp file liên quan; kiểm tra
liên kết chỉ chứng minh đường dẫn còn tồn tại, không chứng minh hình còn đúng.
