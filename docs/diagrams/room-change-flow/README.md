# Sửa và xóa phòng

[Mở sơ đồ HTML](room-change-flow.html) · [Tất cả sơ đồ](../README.md)

## Mục đích

Phân biệt quy tắc cho phép thao tác ở service với việc bảo toàn con trỏ ở kho.

## Cách đọc

- Hình thoi là quyết định; phòng còn suất chiếu, kể cả lịch sử, bị chặn cả sửa/xóa.
- Nhánh sửa: tạo candidate, kiểm tra tên và sinh bộ ghế mới rồi gọi RoomStorage.
- RoomStorage dựng kho ghế tạm, cập nhật phòng, gắn phòng thật vào ghế,
  `relinkSeats()` trên vé rồi `replaceWith()` kho ghế.
- Vé của phòng khác cần relink vì sửa thay toàn bộ kho ghế, làm đổi địa chỉ node.
- Nhánh xóa: xóa riêng ghế thuộc phòng rồi xóa phòng. Ghế phòng khác giữ địa chỉ,
  nên không dựng lại kho và không relink vé.

## Nguồn đối chiếu

[CatalogService.cpp](../../../src/services/CatalogService.cpp): `updateRoom()`, `eraseRoom()`.
[RoomStorage.cpp](../../../src/repositories/RoomStorage.cpp): chuẩn bị và thay kho.
[TicketRepository.cpp](../../../src/repositories/TicketRepository.cpp): `relinkSeats()`.
Điều kiện nghiệp vụ xem [BR-C03/C05 và BR-D02/D03](../../requirements/business-rules.md).

## Giới hạn

Không vẽ riêng lỗi ID, tên, kích thước hoặc cấp phát; hình cũng không có bước
lưu TXT. Chính sách nằm trong BR, không định nghĩa lại bằng hình vẽ.
Đối chiếu source ngày 27/11/2026.
