# Danh mục quy tắc nghiệp vụ

Tài liệu ghi chính sách của rạp mô phỏng đang được ứng dụng thực hiện, không ghi
danh sách hàm hoặc thuật toán. Mã BR dùng để dẫn chiếu từ thiết kế và kiểm thử.

## 1. Phim, phòng và ghế

| Mã | Quy tắc |
|---|---|
| BR-C01 | Phim còn suất tham chiếu được đổi tên nhưng không được đổi thời lượng, kể cả suất lịch sử. |
| BR-C02 | Mỗi phòng có đủ ghế theo số hàng × số ghế mỗi hàng; hàng/cột đánh số từ 1. |
| BR-C03 | Phòng còn suất sử dụng không được sửa, kể cả chỉ đổi tên; không phân biệt suất cũ hay mới. |
| BR-C04 | Không thêm/sửa phim thành tên trùng với phim khác trong danh mục, kể cả phim chỉ còn suất lịch sử. Khi sửa không coi chính phim đó là bản ghi trùng. |
| BR-C05 | Không thêm/sửa phòng thành tên trùng với phòng khác. Khi sửa không coi chính phòng đó là bản ghi trùng; BR-C03 vẫn áp dụng. |

Tên được bỏ dấu cách đầu/cuối rồi so sánh chính xác, phân biệt hoa/thường.
Không gộp dấu cách ở giữa hoặc chuẩn hóa Unicode: ` Film ` trùng `Film`,
nhưng `film` khác `Film`.

## 2. Suất chiếu

| Mã | Quy tắc |
|---|---|
| BR-S01 | Khi thêm/sửa, phim và phòng phải tồn tại; giờ bắt đầu mới phải sau giờ hiện tại. |
| BR-S02 | Cùng phòng không được có hai khoảng sử dụng giao nhau. Khoảng sử dụng là `[bắt đầu, bắt đầu + thời lượng phim + 15 phút nghỉ)`. Tiếp giáp đúng ranh giới được phép; khác phòng được trùng giờ. |
| BR-S03 | Suất đã có vé không được sửa, kể cả đã qua giờ chiếu. |
| BR-S04 | Giờ kết thúc chiếu bằng giờ bắt đầu + thời lượng phim, không gồm thời gian nghỉ. |

Khi sửa, không so trùng lịch với chính suất đó. Suất lịch sử chưa có vé vẫn được
chuyển sang tương lai nếu dữ liệu mới hợp lệ. Không giới hạn khung giờ hoạt động;
suất qua nửa đêm được phép. Chưa áp dụng chính sách giới hạn khoảng giá/bước giá.

## 3. Vé và doanh thu

| Mã | Quy tắc |
|---|---|
| BR-T01 | Ghế phải thuộc phòng của suất và nằm trong kích thước phòng. |
| BR-T02 | Chỉ bán trước giờ bắt đầu; tại đúng giờ bắt đầu cũng bị từ chối. |
| BR-T03 | Mỗi ghế chỉ bán một lần trong một suất; vé ở suất khác không làm ghế đó hết chỗ. |
| BR-T04 | Nhân viên phải xác nhận đã nhận tiền mặt mới được bán. |
| BR-T05 | Vé chốt giá hiện tại của suất và thời điểm bán khi phát hành. |
| BR-T06 | Doanh thu ngày là tổng giá đã chốt của các vé bán trong ngày đó, không phải ngày chiếu. Không có vé thì số vé và doanh thu bằng 0. |

Xem trước không giữ chỗ hoặc tạo vé. Khi xác nhận bán phải kiểm tra lại điều
kiện bán. Tra cứu được vé lịch sử; tên phim/phòng hiển thị là tên hiện tại,
không phải bản chụp lúc bán.

## 4. Quy ước xóa dữ liệu

| Mã | Đối tượng | Quy tắc |
|---|---|---|
| BR-D01 | Phim | Chỉ xóa nếu không còn suất tham chiếu; không xóa kèm suất hoặc vé. |
| BR-D02 | Phòng | Chỉ xóa nếu không còn suất sử dụng; xóa kèm toàn bộ ghế thuộc phòng. |
| BR-D03 | Ghế | Không có thao tác xóa riêng; quản lý theo kích thước và vòng đời phòng. |
| BR-D04 | Suất chiếu | Chỉ xóa nếu chưa có vé, kể cả suất đã qua giờ chiếu. |
| BR-D05 | Vé | Không cung cấp sửa/xóa; giữ để tra cứu và tính doanh thu. |

Quy tắc áp dụng cả lịch sử. Muốn xóa phim/phòng, phải xóa các suất chưa có vé
trước; còn suất có vé thì giữ dữ liệu liên quan. Không chuyển liên kết sang
rỗng, không dùng bản ghi mặc định và không có xóa mềm.

Nơi thực hiện từng BR: [thiết kế service](../design/service-design.md).
Phạm vi kiểm tra khi mở/lưu dữ liệu: [thiết kế TXT](../design/txt-format.md).
