# UML năm entity

[Mở sơ đồ HTML](entities-uml.html) · [Tất cả sơ đồ](../README.md)

## Mục đích

Chỉ ra thuộc tính thể hiện direct association và phân biệt liên kết với sở hữu.

## Cách đọc

- `Seat → Auditorium`; `Screening → Film, Auditorium`; `Ticket → Screening, Seat`.
- Mũi tên đi từ bên giữ con trỏ đến đối tượng được trỏ tới; đây là liên kết một chiều.
- Đầu đích `1`: đúng một đối tượng; đầu còn lại `0..*`: có thể nhiều đối tượng cùng trỏ tới.
- `-` là private, `+` là public. Constructor nhận reference; getter trả reference.
- Repository sở hữu entity; các con trỏ trong entity không sở hữu đối tượng đích.

## Nguồn đối chiếu

[Film.h](../../../src/entities/Film.h),
[Auditorium.h](../../../src/entities/Auditorium.h),
[Seat.h](../../../src/entities/Seat.h),
[Screening.h](../../../src/entities/Screening.h),
[Ticket.h](../../../src/entities/Ticket.h).
Quyền sở hữu và tuổi thọ xem [thiết kế lớp](../../design/class-design.md#2-quan-hệ-giữa-entity).

## Giới hạn

Đủ thuộc tính gốc và năm association; chỉ chọn phương thức tiêu biểu, rút gọn tham
số constructor bằng `…`. Không vẽ service, repository hoặc quan hệ friend.
Không suy ra aggregation/composition từ con trỏ hay chính sách xóa.
Đối chiếu source ngày 27/11/2026.

Bổ sung ngày 28/11/2026: Film có trường dẫn xuất `normalizedTitle_` chỉ để tìm
trên RAM; hình không liệt kê trường kỹ thuật này. Không thay association hoặc
lược đồ TXT. Chi tiết: [thiết kế index tên](../../design/film-name-search.md).
