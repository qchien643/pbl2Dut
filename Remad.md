# Ghi chú ôn tập

Tài liệu ghi nhớ các khái niệm đã dùng trong lõi dữ liệu rạp chiếu phim. Đây là ghi chú học tập, không thay thế tài liệu C++ chính thức.

## 1. Lập trình hướng đối tượng

- **Đóng gói:** entity giữ trạng thái riêng và xác thực dữ liệu khi tạo.
- **Trách nhiệm:** repository quản lý bản ghi/chỉ mục; service thực thi quy tắc liên entity; lớp lưu trữ đọc/ghi dữ liệu.
- **Aggregation:** `ApplicationData` có thể tham chiếu đến các repository được quản lý bên ngoài; vòng đời repository phải dài hơn aggregate.
- **Bất biến:** dữ liệu phải hợp lệ sau mỗi thao tác công khai, không để trạng thái cập nhật dở dang.

## 2. Danh sách liên kết đôi

Mỗi node lưu giá trị và hai liên kết `previous`/`next`. Danh sách giữ con trỏ đầu, cuối và số phần tử.

- Thêm cuối: O(1).
- Tìm theo điều kiện: O(n).
- Xóa khi đã biết node: O(1); tìm node để xóa vẫn tốn O(n).
- Sao chép phải tạo node mới; di chuyển có thể chuyển quyền sở hữu các node.
- Sau khi xóa node, mọi con trỏ đến giá trị trong node đó không còn hợp lệ.

## 3. Bảng băm

Ánh xạ khóa đến bucket bằng hàm băm. Chaining đặt các phần tử cùng bucket vào một danh sách.

- Khóa bằng nhau phải có cùng hash.
- Trung bình tra cứu gần O(1), trường hợp nhiều va chạm có thể là O(n).
- Rehash tạo tập bucket mới rồi mới thay thế dữ liệu cũ để tránh mất trạng thái khi cấp phát thất bại.
- Có thể dùng hash tùy chỉnh cho ID, ngày hoặc khóa ghép.

## 4. Ma trận hai chiều

Ma trận lưu số hàng, số cột và vùng ô liên tục. Chỉ số tuyến tính của `(row, column)` là `row * columns + column`. Trước khi cấp phát phải kiểm tra phép nhân kích thước có tràn hay không; mọi truy cập cần kiểm tra biên.

## 5. Quy tắc an toàn cần nhớ

- Kiểm tra ID, thời lượng, kích thước phòng và giá vé dương.
- Kiểm tra tràn khi nhân số ghế, cấp ID, đếm vé và cộng doanh thu.
- Khi cập nhật nhiều chỉ mục, dùng dữ liệu tạm hoặc rollback nếu có lỗi.
- Test cả dữ liệu rỗng, biên, va chạm, copy/move và trường hợp lỗi.
