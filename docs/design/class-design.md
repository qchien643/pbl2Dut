# Thiết kế lớp và quan hệ đối tượng

Cập nhật: 27/10/2026 cho nhánh thử nghiệm `experiment/direct-association`.
Đọc cùng [thiết kế dữ liệu](data-design.md) và [hướng dẫn vấn đáp](../study-guide.md).

## 1. Phân chia trách nhiệm

| Nhóm | Dữ liệu/trách nhiệm | Không làm |
|---|---|---|
| `entities/` | Năm lớp dữ liệu; constructor kiểm tra giá trị và giữ association trực tiếp | Không đọc file, không gọi Qt |
| `DataStructures/` | Danh sách, bảng băm, ma trận dùng template | Không biết phim, phòng hay quy tắc bán vé |
| `repositories/` | Lưu bản ghi trên RAM, tìm ID, đồng bộ danh sách/chỉ mục, đánh dấu thay đổi | Không bảo vệ đầy đủ quy tắc liên thực thể |
| `services/` | Phối hợp repository, kiểm tra nghiệp vụ | Không ghi TXT, không tạo widget |
| `persistence/` | Chuyển một repository sang/từ một file TXT | Không tự thực hiện giao dịch trên cả bộ file |
| `ApplicationData` | Sở hữu năm repository; nạp, kiểm tra và lưu cả bộ | Không quyết định người dùng có xác nhận tiền hay không |
| `ui/` | Nhập/hiển thị dữ liệu, xác nhận, gọi service và nạp/lưu | Không dùng repository trực tiếp |

Luồng thông thường: `MainWindow → Service → Repository → DSA`.
Luồng lưu: `MainWindow → ApplicationData → TxtPersistence → TXT`.
`main.cpp` tạo và nối các đối tượng; không phải nơi cài đặt nghiệp vụ.

## 2. OOP thể hiện ở đâu?

| Khái niệm | Bằng chứng trong dự án | Cần phân biệt |
|---|---|---|
| Đóng gói | Thuộc tính `Film` là `private`; getter đọc; constructor kiểm tra dữ liệu | Không có setter không có nghĩa đối tượng hoàn toàn bất biến |
| Trừu tượng hóa | `sell()` che các bước tra suất, kiểm tra ghế, tạo vé; `findById()` che cách băm | Không nhất thiết phải có lớp trừu tượng |
| Kế thừa | `FilmRepository : public Repository<Film>`; tương tự phòng, ghế, suất | Kế thừa ở đây tái sử dụng code, không có hàm nghiệp vụ `virtual` |
| Template | `Repository<T>`, `LinkedList<T>`, `HashTable<Key, Value, Hasher>`, `Matrix2D<T>` | Kiểu cụ thể được xác định khi biên dịch, không phải chọn lớp qua `virtual` |
| Nạp chồng | `find()` bản thường/bản `const`; các `parse()` cho từng entity | Nạp chồng khác ghi đè |
| Ghi đè/đa hình động | `MainWindow::closeEvent(...) override`; `MoneyItem::operator<` ghi đè hàm Qt | Phần lõi nghiệp vụ không có hệ phân cấp đa hình động tự xây |
| Hợp thành | `ApplicationData` chứa repository; repository chứa danh sách | Xét quyền sở hữu và vòng đời, không chỉ tên lớp |

Không tạo lớp cha `Entity` hoặc hàm thuần ảo chỉ để có đủ thuật ngữ OOP.
Năm entity không cần giao diện `virtual` chung; template dùng được với các kiểu
cung cấp những thao tác mà nó cần.

## 3. Quan hệ đối tượng: ai sở hữu ai?

| Quan hệ | Code tiêu biểu | Phân loại và vòng đời |
|---|---|---|
| `FilmRepository` → `Repository<Film>` | `: public Repository<Film>` | Kế thừa công khai; không phải với `Repository<Ticket>` |
| `TicketRepository` → `Repository<Ticket>` | `Repository<Ticket> records_;` | Composition: thành viên tạo/hủy cùng đối tượng chứa |
| `ApplicationData` → năm repository | `FilmRepository films;` và bốn thành viên khác | Composition; hủy `ApplicationData` sẽ hủy chúng |
| Repository → danh sách, bảng băm | `records_`, `index_` | Composition đối với hai container |
| Danh sách → entity | Node chứa `T value;` | Value container: sở hữu bản sao entity |
| Chỉ mục → entity | Entry chứa giá trị `T*` | Tham chiếu không sở hữu; bảng băm không `delete` entity |
| Service → repository | `FilmRepository& films_;` | Association không sở hữu; repository sống lâu hơn service |
| `MainWindow` → service/data | Các thành viên tham chiếu | Association không sở hữu; cửa sổ không hủy chúng |
| `Seat` → `Auditorium` | `const Auditorium* auditorium_;` | Association trực tiếp, không sở hữu |
| `Screening` → `Film`, `Auditorium` | Hai con trỏ `const` | Association trực tiếp, không sở hữu |
| `Ticket` → `Screening`, `Seat` | Hai con trỏ `const` | Association trực tiếp, không sở hữu |
| Service → helper | Gọi `screening_time::endMinute(...)` | Dependency: sử dụng hàm hỗ trợ |
| Qt parent → widget con | `new QLabel(parent)` | Qt quản lý vòng đời widget con qua parent |

Theo cách phân loại container trong `ObjectRelation.pdf`, danh sách giữ giá trị
là value container; chỉ mục chứa con trỏ là reference container, có thể mô tả
quan hệ với entity là aggregation không sở hữu. Với service, nói
“association bằng tham chiếu, không sở hữu” chính xác hơn việc gọi mọi dấu `&`
là aggregation. Quan hệ “toàn thể–bộ phận” phải có ý nghĩa mô hình.

Ở mức dữ liệu, một phòng có nhiều ghế. Nhưng lớp `Auditorium` **không chứa**
`LinkedList<Seat>`; `SeatRepository` mới sở hữu các đối tượng ghế.
Không đồng nhất ERD với sơ đồ sở hữu bộ nhớ.

Entity liên quan giữ con trỏ tới đối tượng thật trong repository. Getter ID vẫn
có để ghi TXT, nhưng ID được lấy từ đối tượng đích, không lưu thành thuộc tính
thứ hai. Vì vậy repository sở hữu phải sống lâu hơn entity tham chiếu và không
được xóa đối tượng đích khi còn quan hệ. Service thực thi quy tắc vòng đời này.

## 4. Entity: tính hợp lệ của một đối tượng

Mỗi entity có cặp `.h/.cpp` riêng dưới `src/entities/`.
`EntityId.h` đặt bí danh cho `std::int64_t`; không tạo một lớp ID mới.

| Lớp | Constructor kiểm tra | Điều phải kiểm tra ở nơi khác |
|---|---|---|
| `Film` | ID, thời lượng dương; tên hợp lệ | Có suất tham chiếu hay chưa |
| `Auditorium` | ID, kích thước dương; tên; tích kích thước không tràn | Đủ ghế và không có suất khi sửa/xóa |
| `Seat` | ID/vị trí dương; nhận một `Auditorium&` hợp lệ | Vị trí nằm trong phòng và duy nhất |
| `Screening` | ID, ngày giờ, giá; nhận `Film&` và `Auditorium&` | Không trùng lịch; tương lai khi tạo/sửa |
| `Ticket` | ID, ngày giờ, giá; nhận `Screening&`, `Seat&`; ghế cùng phòng | Không trùng suất-ghế, bán trước suất |

`EntityValidation` chứa hàm tự do trong namespace, không phải lớp cha.
Entity không có constructor mặc định để tránh tạo bản ghi thiếu dữ liệu.
Destructor do compiler sinh đủ dùng vì các con trỏ association không sở hữu.
Copy entity sao chép con trỏ, nên bản sao vẫn liên kết đến cùng đối tượng đích.

Getter `const std::string& title() const` không sao chép chuỗi và không cho sửa
qua tham chiếu trả về. Tham chiếu vẫn phụ thuộc vòng đời entity.
Entity còn phép gán mặc định; bảo vệ ID đang lưu chủ yếu nằm ở API chỉ đọc của
repository và kiểm tra `id == value.id()` khi cập nhật.

## 5. Repository: sở hữu và chỉ mục

`Repository<T>` chứa:

```cpp
LinkedList<T> records_;
HashTable<EntityId, T*> index_;
EntityId highestId_ = 0;
bool dirty_ = false;
```

Danh sách sở hữu bản ghi, bảng băm lưu địa chỉ bản ghi để tìm theo mã.
Bất biến: mỗi bản ghi có đúng một entry ID trỏ tới nó; không có entry trỏ vào
đối tượng đã hủy. Rehash bảng băm thay entry chỉ mục, không di chuyển entity
trong danh sách.

| Hàm | Ý nghĩa và trường hợp lỗi |
|---|---|
| `empty`, `size` | Đọc số bản ghi, không đọc file |
| `nextId` | Trả `highestId_ + 1`, kiểm tra tràn; chưa giữ chỗ hoặc tăng bộ đếm |
| `findById` | Trả `const T*`; không thấy trả `nullptr` |
| `insert` | Từ chối ID trùng; thêm list rồi index; lỗi index thì gỡ node mới |
| `update` | Không đổi ID; không thấy trả `false`; copy trước rồi swap để giữ bản cũ nếu copy lỗi |
| `erase` | Không thấy trả `false`; xóa index trước rồi xóa đúng địa chỉ trong list |
| `begin`, `end` | Chỉ cung cấp duyệt đọc, kể cả repository không `const` |
| `replaceWith` | Hoán đổi list và index cùng nhau; giữ mốc ID cũ nếu cao hơn; đánh dấu dirty |
| `isDirty`, `markSaved` | Đọc/xóa cờ thay đổi; không tự so sánh hoặc ghi dữ liệu trên đĩa |

`replaceWith()` chuyển trạng thái cũ sang đối tượng tạm; khi đối tượng tạm hủy,
dữ liệu cũ mới được giải phóng. Vì `Ticket` trỏ trực tiếp tới `Seat`, thao tác
tạo lại kho ghế cũng dựng lại kho vé để mọi vé trỏ tới node ghế mới trước khi
dữ liệu cũ bị hủy. UI vẫn giữ ID và tra lại, không giữ con trỏ lâu dài.

### Vì sao cấm copy/move?

Copy mặc định container sở hữu con trỏ sẽ sao chép địa chỉ, gây dùng chung tài
nguyên và nguy cơ xóa hai lần. Ngay cả nếu list có deep copy, chỉ mục vẫn cần
dựng lại để trỏ vào bản ghi của bản sao. Chưa có nhu cầu copy/move repository,
nên `= delete` ngăn dùng sai; `swap()` hỗ trợ trường hợp đổi bộ dữ liệu cần thiết.

`Repository` không có destructor `virtual`. Nó không được dùng để sở hữu và
`delete` đối tượng lớp dẫn xuất qua `Repository<T>*`. Kế thừa công khai không tự
biến một lớp thành lớp cơ sở đa hình.

### Vì sao TicketRepository không kế thừa?

Bốn repository còn lại kế thừa để dùng CRUD chung. `TicketRepository` chứa
`Repository<Ticket>` và chỉ chuyển tiếp thêm/tìm/duyệt/trạng thái lưu.
Không công khai `update`, `erase`, `replaceWith`: tránh cung cấp thao tác sửa/xóa
vé đã bán. Các hàm chuyển tiếp không cài lại thuật toán.

`friend class ApplicationData` cho phép thay dữ liệu khi nạp; `CatalogService`
được phép nối lại kho vé với kho ghế mới. Việc chỉ đổi con trỏ không làm thay
đổi dữ liệu TXT nên trạng thái dirty trước đó của vé được giữ nguyên.
Composition ở đây hạn chế API; vẫn không tự kiểm tra tiền mặt hoặc khóa ngoại.

## 6. Service: đặt quy tắc đúng nơi

| Service | Repository được sửa | Nhiệm vụ chính |
|---|---|---|
| `CatalogService` | Phim, phòng, ghế; nối lại vé khi thay kho ghế | CRUD danh mục; chặn dữ liệu đang được tham chiếu; chuẩn bị dữ liệu mới trước khi thay |
| `ScreeningService` | Suất | Kiểm tra phim/phòng, lịch phòng, tương lai, vé tham chiếu |
| `TicketService` | Vé | Xem trước, bán, tìm vé, dựng sơ đồ, lọc vé và tính doanh thu |

Repository chỉ đọc được truyền bằng `const ...&`; repository cần cập nhật
bằng `...&`. Service không sở hữu chúng. Danh sách khởi tạo constructor gắn các
tham chiếu ngay khi tạo, không phải sao chép repository.

Kết quả lọc được chép vào `LinkedList` tạm rồi `swap(result)` sau khi thành công.
Kết quả là bản sao độc lập; lỗi giữa chừng không làm mất kết quả cũ.
`SaleDetails`, `TicketDetails`, `DailyRevenue`, `SeatStatus` là kiểu kết quả tạm,
không phải thêm bảng nghiệp vụ.

`ServiceClock` là con trỏ hàm trả chuỗi giờ hiện tại; mặc định dùng
`localMinuteNow`. Test truyền đồng hồ cố định để không phụ thuộc ngày chạy.
Không phải interface đồng hồ có hàm ảo.

## 7. Bộ nhớ và chi phí cần biết

Gọi `n` là số bản ghi, `s` là tổng ghế, `t` là tổng vé, `k` là số suất.
Chi phí bảng băm trung bình giả định phân bố khóa hợp lý.

| Thao tác | Chi phí/giới hạn thực tế |
|---|---|
| `LinkedList::pushBack` | O(1), cấp phát một node |
| Duyệt/find/erase theo địa chỉ trong list | O(n); nối lại con trỏ sau khi tìm được node mới là O(1) |
| Tìm ID trong repository | Trung bình O(1), xấu nhất O(n) |
| Insert hash | Trung bình khấu hao O(1); lần rehash phải duyệt entries/buckets |
| Erase repository | O(n) do list phải tìm node bằng địa chỉ |
| Truy cập ô ma trận | O(1); bộ nhớ O(hàng × cột) |
| Kiểm tra trùng lịch một suất | O(k); thời lượng lấy trực tiếp từ `Screening::film()` |
| Kiểm tra toàn bộ lịch lúc nạp/lưu | O(k²), do mỗi suất đối chiếu các suất khác |
| Kiểm tra ghế đã bán / doanh thu | O(t) |
| Dựng sơ đồ suất | O(s + t + số ô), với tra ID trung bình O(1) |
| Tạo lại ghế phòng | Dựng kho ghế và vé tạm để nối lại con trỏ; tốn thêm bộ nhớ theo tổng ghế + vé |

`unique_ptr<SeatMap>` sở hữu ma trận tạm và tự hủy khi ra khỏi phạm vi.
Trong Qt, parent hoặc container như `QTableWidget` nhận quyền sở hữu widget/item
theo thao tác tạo/thêm. Không thể kết luận “mọi `new` đều tự được Qt dọn”.

## 8. Giới hạn phải nói thật khi vấn đáp

- Core dùng template, overload và composition; không có lớp abstract nghiệp vụ.
- Các repository trong `ApplicationData` là public. Code gọi trực tiếp có thể
  bỏ qua service; đây không phải hàng rào bảo mật. `validate()` kiểm tra lại trước lưu.
- Dirty chỉ ghi nhận thao tác thành công, không chứng minh RAM khác file ở từng byte.
- Lưu nhiều TXT có phục hồi lỗi, không phải transaction ACID đầy đủ.
- Không có khóa cho hai tiến trình; kiểm tra lại lúc bán không giải quyết cạnh tranh đa phiên.
- Không có `inline` viết trực tiếp trong `src`, nhưng hàm định nghĩa trong thân
  lớp vẫn có ngữ nghĩa inline của C++; không đồng nghĩa compiler bắt buộc chèn mã.

Thứ tự sống trong `main`: tạo `data`, các service, rồi `window`; khi rời scope,
hủy theo thứ tự ngược lại. Nhờ đó tham chiếu trong service và cửa sổ còn hợp lệ.
Destructor dọn RAM, không thay thế thao tác lưu dữ liệu.
