# Trình tự bán một vé

[Mở sơ đồ HTML](ticket-sale-sequence.html) · [Tất cả sơ đồ](../README.md)

## Mục đích

Lần theo `sell()` từ UI, qua kiểm tra lại, đến thêm vé trong kho RAM và trả mã vé.

## Cách đọc

- Thời gian đi từ trên xuống; đường liền là gọi, đường đứt là trả về.
- UI xác nhận nhận tiền rồi gọi `sell(..., true)`. Preview trước đó không giữ chỗ.
- Service tìm suất/ghế, lấy giờ và kiểm tra lại điều kiện bán; không tin preview cũ.
- Tiếp theo lấy ID, giá từ suất, tạo Ticket với các con trỏ gốc và gọi `insert()`.
- Repository lưu bản sao trong LinkedList, cập nhật index và dirty; UI nhận mã vé.
- Chưa ghi TXT. Lưu file chỉ diễn ra ở bước lưu/thoát ứng dụng.

## Nguồn đối chiếu

[CinemaWindow.cpp](../../../src/gui/CinemaWindow.cpp): `sellTicket()`.
[TicketService.cpp](../../../src/services/TicketService.cpp): `sell()`, `details()`, `requireAvailable()`.
[Repository.h](../../../src/repositories/Repository.h): `insert()` và rollback.
Quy tắc và ánh xạ code xem [thiết kế service](../../design/service-design.md#3-ticketservice).

## Giới hạn

Chỉ vẽ nhánh thành công; hai kho đọc được gộp, bỏ các lần tìm lại và lời gọi
helper. Chi tiết thứ tự kiểm tra xem source. Lỗi được UI bắt và báo; insert lỗi
cấp phát được rollback. `sell()` hiện không dùng bool trả về từ `insert()`.
Đối chiếu source ngày 27/11/2026.
