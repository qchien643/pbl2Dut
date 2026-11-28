# Vòng đời TXT → RAM → TXT

[Mở sơ đồ HTML](txt-lifecycle.html) · [Tất cả sơ đồ](../README.md)

## Mục đích

Theo dõi dữ liệu được tạo ở đâu, ai sở hữu nó và khi nào mới ghi kết quả ra file.

## Cách đọc

Trang có ba hình chi tiết, mỗi hình tám bước đánh số, cùng một phần giải thích RAM.
Đọc hàng trên từ trái sang phải, xuống hàng dưới rồi đi từ phải sang trái.
Nhãn chính và diễn giải dùng tiếng Việt; tên hàm chỉ để đối chiếu code.

**Nguyên tắc nạp:** bảng được tham chiếu nạp xong trước bảng giữ tham chiếu,
để có thể tìm đối tượng theo ID và lấy địa chỉ khi tạo liên kết trên RAM.
Code hiện nạp **Phim → Phòng → Ghế → Suất chiếu → Vé**:
phim/phòng không tham chiếu; ghế cần phòng; suất chiếu cần phim và phòng;
vé cần suất chiếu và ghế. Thứ tự này được viết trong `CinemaDatabase::load()`,
không tự suy ra từ ID hay thứ tự cột. Các bảng độc lập có thể đổi thứ tự,
miễn luôn đáp ứng các phụ thuộc trên.

1. **Nạp toàn bộ:** tạo kho rỗng, kiểm tra file, tạo kho tạm, nạp năm bảng,
   kiểm tra toàn vẹn, chuyển node sang kho chính, đánh dấu sẵn sàng, mở Qt.
   Bảng phụ ghi thứ tự Film → Auditorium → Seat → Screening → Ticket và các nhánh lỗi.
2. **Theo dõi một dòng ghế:** mở TXT, đọc tên cột, tạo bảng chuỗi, đổi ID và tìm phòng,
   tạo Seat, chép vào node, lập chỉ mục, đọc tiếp và giải phóng dữ liệu tạm.
   Ví dụ `17|2|2|5` giải thích ID, con trỏ và vòng đời biến cục bộ.
3. **Dữ liệu trên RAM:** phân biệt danh sách sở hữu entity, chỉ mục giữ địa chỉ,
   con trỏ association và cờ chưa lưu. Giải thích vì sao chuyển kho không làm sai liên kết.
4. **Lưu và thoát:** xác nhận, kiểm tra, chuẩn bị vùng tạm, ghi/đóng đủ năm file,
   sao lưu, thay file chính, giữ backup, đánh dấu đã lưu rồi đóng cửa sổ.
   Bảng phụ giải thích ghi con trỏ thành ID và phục hồi khi lỗi.

Chữ chính trong hình 16px; màn hình hẹp cuộn ngang từng hình, không thu nhỏ chữ.

## Nguồn đối chiếu

[TextFile.cpp](../../../src/fileio/TextFile.cpp),
[EntityFile.cpp](../../../src/fileio/EntityFile.cpp),
[CinemaDatabase.cpp](../../../src/database/CinemaDatabase.cpp).
Quyền sở hữu đối tượng xem [Repository.h](../../../src/repositories/Repository.h);
liên kết ghế xem [Seat.cpp](../../../src/entities/Seat.cpp);
luồng đóng cửa sổ xem [CinemaWindow.cpp](../../../src/gui/CinemaWindow.cpp).
Định dạng cột và phục hồi xem [thiết kế TXT](../../design/txt-format.md).

## Giới hạn

Khoảng cách giữa bước chỉ biểu diễn thứ tự, không phải thời lượng. Nạp/lưu
không kiểm tra lại mọi quy tắc nghiệp vụ tạo/sửa. Ba hình vẽ luồng thành công;
nhánh rỗng, file thiếu và lỗi phục hồi được ghi trong các bảng ngay trên trang HTML.
Không vẽ từng lệnh hệ thống hay điểm chèn lỗi dành cho kiểm thử;
không suy ra giao dịch nguyên tử hoặc khả năng chống mọi sự cố từ hình.
Đối chiếu source ngày 27/11/2026.
