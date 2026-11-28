# Repository, service và DI/DIP

[Mở sơ đồ HTML](repository-service-uml.html) · [Tất cả sơ đồ](../README.md)

## Mục đích

Giải thích vì sao service nhận interface nhưng vẫn dùng được repository cụ thể.

## Cách đọc

- Đường đứt, tam giác rỗng: `Repository<T>` triển khai `IRepository<T>`.
- Đường liền, tam giác rỗng: `TicketRepository` kế thừa `Repository<Ticket>`.
- Hình thoi đặc ở Repository: nó sở hữu LinkedList và HashTable thành viên.
- LinkedList giữ entity; HashTable giữ `T*` trỏ tới entity đó, không sở hữu entity.
- Dependency từ TicketService minh họa kho vé với `T = Ticket`. Bảng dưới HTML
  liệt kê đủ dependency của cả ba service, bao gồm `IRoomStorage` và đồng hồ.

DI là truyền dependency qua constructor ở `main.cpp`; DIP là service phụ thuộc
interface thay vì cách cài đặt kho. Hai khái niệm không đồng nghĩa.

## Nguồn đối chiếu

[IRepository.h](../../../src/repositories/IRepository.h),
[Repository.h](../../../src/repositories/Repository.h),
[TicketService.h](../../../src/services/TicketService.h),
[main.cpp](../../../src/main.cpp),
[DependencyInjectionTest.cpp](../../../tests/DependencyInjectionTest.cpp).
Chi tiết xem [thiết kế lớp](../../design/class-design.md#5-service).

## Giới hạn

Chỉ vẽ một service và một kho cụ thể làm ví dụ; rút gọn một số hàm, tham số.
Không có DI container hay interface cho toàn hệ thống. `CinemaDatabase` vẫn
biết lớp cụ thể để quản lý vòng đời. Đối chiếu source ngày 27/11/2026.

Bổ sung ngày 28/11/2026: CatalogService nhận một `IFilmRepository&` cho cả CRUD
và tìm tên, không truyền kho phim hai lần. Sơ đồ ví dụ kho vé giữ nguyên;
ScreeningService cũng nhận `const IFilmRepository&` theo quy ước dùng thống nhất
interface chuyên biệt trong service, không phụ thuộc việc có gọi search.
`Repository<T>` hiện dùng `public virtual IRepository<T>` để nhánh phim chia sẻ
base CRUD với IFilmRepository; hình chưa bổ sung nhãn kế thừa ảo. Chi tiết nhánh phim xem
[thiết kế index tên](../../design/film-name-search.md).
