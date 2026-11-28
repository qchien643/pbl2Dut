# Đặc tả yêu cầu phần mềm (SRS)

Tài liệu xác định sản phẩm phải làm gì và các ràng buộc chất lượng; không mô tả
hàm C++, thuật toán hay định dạng từng cột TXT.

## 1. Phạm vi

Ứng dụng quản lý một rạp phim mô phỏng, dành cho một nhân viên, chạy offline
trên một máy và một phiên. Quản lý phim, phòng, ghế, suất chiếu, vé và doanh thu.

Ngoài phạm vi: tài khoản, đặt vé online, giữ chỗ, hủy/hoàn vé, thanh toán điện tử,
khuyến mãi, nhiều phiên đồng thời và hệ quản trị SQL.

## 2. Yêu cầu chức năng

| Mã | Yêu cầu |
|---|---|
| FR-01 | Thêm, sửa, xóa phim; tìm theo mã; tìm tên có/không dấu, không phân biệt hoa/thường, không phụ thuộc thứ tự từ, hỗ trợ tiền tố/chuỗi con và lỗi gõ theo phạm vi bên dưới; xem danh sách phim. |
| FR-02 | Thêm, sửa, xóa phòng; tạo và hiển thị sơ đồ ghế theo kích thước phòng. |
| FR-03 | Thêm, sửa, xóa suất; tìm theo mã, lọc theo ngày/phim/phòng; chỉ liệt kê suất chưa bắt đầu khi chọn suất để bán; hiển thị giờ kết thúc. |
| FR-04 | Chọn suất và ghế, xem trước thông tin, xác nhận nhận tiền mặt và phát hành vé. |
| FR-05 | Tra vé theo mã, liệt kê vé theo ngày bán và hiển thị số vé/doanh thu ngày. |
| FR-06 | Nạp bộ dữ liệu khi mở; hỗ trợ bắt đầu từ thư mục rỗng sau khi xác nhận. |
| FR-07 | Giữ thay đổi trong phiên và ghi toàn bộ khi lưu/thoát; báo lỗi nếu không thể nạp hoặc lưu. |

Các thao tác phải tuân theo [Danh mục quy tắc nghiệp vụ](business-rules.md).
Phạm vi dữ liệu nạp/lưu được xác định trong [thiết kế TXT](../design/txt-format.md).

### Tìm tên phim thuộc FR-01

Dùng **Nguyễn Trãi** làm tên phim minh họa để đối chiếu đúng ví dụ đã thống nhất:

| Input | Kết quả cần đạt |
|---|---|
| `nguyen trai` | Tìm được tên có dấu `Nguyễn Trãi`; nhập có dấu/hoa thường cũng khớp. |
| `trai nguyen` | Tìm được dù đảo thứ tự từ. |
| `nguy` | Tìm được bằng tiền tố của một từ trong tên. |
| `guy` | Tìm được bằng chuỗi nằm giữa từ. |
| `nguyn trai` | Tìm được dù thiếu chữ `e` trong `nguyen`. |

Mọi từ tìm phải được đáp ứng trong tên, không bắt buộc thứ tự. Phạm vi: từ tìm
có ít nhất ba ký tự; hỗ trợ lỗi một thao tác thêm/xóa/thay/đảo hai ký tự liền
nhau trên nguyên từ khi từ nhập và từ đúng còn đoạn ba ký tự chung. Không cam
kết mọi lỗi chính tả, đồng nghĩa hoặc tìm theo ngữ nghĩa. Từ nhập dưới ba ký tự
được báo yêu cầu nhập thêm; ô trống vẫn xem tất cả. Tên hiển thị/lưu trữ không
đổi; không thay quy tắc trùng tên.

Kết quả tìm phải phản ánh ngay việc thêm, sửa tên hoặc xóa phim thành công trong
phiên. Cách xây và đồng bộ index nằm ở [thiết kế index tên](../design/film-name-search.md);
bằng chứng triển khai thử nằm trong [benchmark](../testing/film-search-benchmark.md).

## 3. Yêu cầu chất lượng và ràng buộc môn học

| Mã | Yêu cầu |
|---|---|
| NFR-01 | C++17, giao diện Qt 6 Widgets, cấu hình bằng CMake; không cần kết nối mạng khi sử dụng. |
| NFR-02 | Kho entity chính dùng LinkedList và HashTable tự cài đặt; sơ đồ ghế dùng Matrix2D tự cài đặt. |
| NFR-03 | Đầu vào sai hoặc thao tác bị từ chối phải được báo lỗi; không để lại thay đổi dữ liệu một phần. |
| NFR-04 | Lưu lỗi phải giữ dữ liệu trong phiên, cho phép thử lại và báo rõ nếu cần phục hồi thủ công; không làm mất dữ liệu âm thầm. |
| NFR-05 | Sau khi lưu thành công và mở lại, dữ liệu và các liên kết phải được giữ đúng. |

## 4. Nghiệm thu và tài liệu liên quan

Kiểm chứng FR/NFR bằng các bộ test và phạm vi đối chiếu trong
[kế hoạch kiểm thử](../testing/verification.md). Test không thay thế kiểm tra
thao tác thực tế trên giao diện.

- Cách thao tác: [hướng dẫn sử dụng](../guides/user-guide.md).
- Hành vi từng tab, cách tìm/lọc và giới hạn: [feature map](../design/ui-feature-map.md).
- Quyết định nghiệp vụ: [business rules](business-rules.md).
- Cấu trúc dữ liệu sản phẩm: [thiết kế dữ liệu](../design/data-design.md).
- Cách triển khai: [thiết kế lớp](../design/class-design.md) và
  [thiết kế service](../design/service-design.md).
