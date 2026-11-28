# Kiến trúc hệ thống

[Mở sơ đồ HTML](system-architecture.html) · [Tất cả sơ đồ](../README.md)

## Mục đích

Xác định trách nhiệm từng tầng và nơi ghép dependency khi chạy chương trình.

## Cách đọc

- `main.cpp` tạo database, nạp dữ liệu, tạo ba service rồi truyền vào cửa sổ.
- Qt gọi service; service kiểm tra nghiệp vụ và thao tác repository trên RAM.
- Service gọi helper kỹ thuật trực tiếp: SeatMap dựng ma trận, DateTimeHelper xử lý
  thời gian. Helper không phải service thứ tư và không cần thuộc tính DI mới.
- CatalogService kiểm tra sơ đồ đủ ghế trong `makeRoomSeatMap()`; TicketService
  dùng lại hàm này rồi đánh dấu ghế đã bán theo suất. Helper không chứa BR-C02.
- Database sở hữu kho và điều phối fileio khi nạp/lưu. Service không ghi TXT.
- Mũi tên biểu diễn hướng gọi/quản lý, không phải ký hiệu kế thừa UML.

## Nguồn đối chiếu

- [main.cpp](../../../src/main.cpp): khởi tạo và constructor injection.
- [CinemaDatabase.h](../../../src/database/CinemaDatabase.h): sở hữu các kho.
- [CinemaWindow.cpp](../../../src/gui/CinemaWindow.cpp): UI gọi service và lưu/thoát.
- [SeatMap.cpp](../../../src/helper/SeatMap.cpp): dựng ma trận từ danh sách ghế.
- [CatalogService.cpp](../../../src/services/CatalogService.cpp): quy tắc sơ đồ phòng đầy đủ.
- [Thiết kế lớp](../../design/class-design.md): phân tầng và vòng đời đầy đủ.

## Giới hạn

Gộp ba service, năm repository và các helper; không vẽ từng hộp thoại hay nhánh lỗi.
Chi tiết DI/DIP xem [UML repository–service](../repository-service-uml/README.md).
Đối chiếu source ngày 27/11/2026; khi dependency đổi, cập nhật cả hình và mô tả.
