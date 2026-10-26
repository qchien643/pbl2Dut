# Hướng dẫn đọc code và vấn đáp 1:1

Cập nhật: 26/10/2026. Dùng cùng [thiết kế lớp](design/class-design.md) và
[thiết kế dữ liệu](design/data-design.md). Các đoạn trích lấy từ code hiện tại;
đoạn minh họa được ghi rõ. Không học thuộc câu trả lời thay cho chạy và đọc code.

## 1. Khi thầy chỉ vào một dòng

Trả lời đủ năm ý, tùy loại dòng:

1. Kiểu dữ liệu là gì? Đối tượng, con trỏ, tham chiếu hay bản sao?
2. Dòng này làm gì? Đầu vào/đầu ra và trạng thái nào thay đổi?
3. Ai sở hữu dữ liệu? Khi nào dữ liệu hoặc địa chỉ đó hết hiệu lực?
4. Vì sao cần dòng này? Bỏ đi thì lỗi gì hoặc mất quy tắc nào?
5. Có thể thất bại không? Ném exception, trả `false` hay `nullptr`; trạng thái cũ còn không?

Tài liệu giải thích các mẫu và đường chạy chính, không chú thích lại mọi dòng
widget giống nhau. Mục tiêu cuối là tự mở bất kỳ hàm nào và áp dụng năm câu trên.

## Mục lục học

- [Lộ trình và bản đồ file](#2-lộ-trình-và-bản-đồ-file)
- [Entity](#3-entity-đọc-từ-khai-báo-đến-constructor)
- [Cấu trúc dữ liệu](#4-dsa-biết-từng-con-trỏ-trỏ-vào-đâu)
- [Repository](#5-repository-đọc-những-dòng-khó)
- [Service](#6-service-ba-luồng-phải-tự-lần-được)
- [TXT và ApplicationData](#7-txt-và-applicationdata)
- [Qt, main và build](#8-qt-main-và-hệ-thống-build)
- [Cú pháp cần biết](#9-những-từ-khóa-còn-lại-không-được-né)
- [Câu hỏi tự vấn đáp](#10-bộ-câu-hỏi-tự-vấn-đáp)
- [Thực hành](#11-thực-hành-để-biết-mình-thực-sự-hiểu)

## 2. Lộ trình và bản đồ file

Mỗi hàng là một chặng. Chỉ chuyển tiếp khi trả lời được cột cuối mà không đọc đáp án.
Đường dẫn dưới đây tính từ `src/`; `{h,cpp}` nghĩa là đọc cả hai file.

| Chặng | File | Phải tự giải thích được |
|---|---|---|
| 1 | `entities/EntityId.h`, `Film.{h,cpp}`, `EntityValidation.{h,cpp}` | Kiểu ID, đóng gói, constructor, getter, exception, kiểm tra ngày/tên |
| 2 | `entities/Auditorium.{h,cpp}`, `Seat.{h,cpp}`, `Screening.{h,cpp}`, `Ticket.{h,cpp}` | Khác biệt thuộc tính/ràng buộc; FK dương chưa có nghĩa FK tồn tại |
| 3 | `DataStructures/LinkedList.h` | Node, đầu/cuối, ownership, thêm/xóa, destructor và iterator |
| 4 | `DataStructures/HashTable.h`, `Matrix2D.h` | Va chạm, rehash, con trỏ giá trị, ánh xạ ô và kiểm tra tràn |
| 5 | `repositories/Repository.h` | Template, list/index, `T**`, dirty, ID, rollback |
| 6 | `FilmRepository.h`, `AuditoriumRepository.h`, `SeatRepository.h`, `ScreeningRepository.h`, `TicketRepository.{h,cpp}` trong `repositories/` | Kế thừa so với composition, API vé bị giới hạn |
| 7 | `services/CatalogService.{h,cpp}`, `SeatMap.{h,cpp}` | Tham chiếu, sinh ghế, sửa phòng không dở dang, ma trận tạm |
| 8 | `services/ScreeningTime.{h,cpp}`, `ScreeningValidation.{h,cpp}`, `ServiceClock.{h,cpp}`, `ScreeningService.{h,cpp}` | Tính phút, khoảng nửa mở, lịch sử so với tạo mới, đồng hồ test |
| 9 | `services/TicketService.{h,cpp}` | Xem trước khác bán thật, giá chốt, tra liên kết, doanh thu |
| 10 | `persistence/TxtPersistence.{h,cpp}`, `ApplicationData.{h,cpp}` | Overload, parser, kiểm tra toàn bộ, nạp tạm, lưu và phục hồi |
| 11 | `main.cpp`, `ui/MainWindow.{h,cpp}` | Thứ tự sống, event loop, signal/lambda, ownership Qt, đóng cửa sổ |
| 12 | `../CMakeLists.txt`, `../tests/*Test.cpp` | Biên dịch/liên kết, assert, test lỗi và giới hạn bằng chứng |

Các chặng 1–6 là nền OOP; chặng 7–10 nối OOP với thiết kế dữ liệu.
Không bỏ qua Qt và kiểm thử nếu yêu cầu là hiểu toàn bộ code tự viết.

## 3. Entity: đọc từ khai báo đến constructor

Ví dụ thật trong `Film.h`:

```cpp
Film(EntityId id, std::string title, std::int64_t durationMinutes);
EntityId id() const;
const std::string& title() const;
```

| Cụm cú pháp | Cách giải thích |
|---|---|
| `Film(...)` | Constructor cùng tên lớp, không có kiểu trả về; có thể ném lỗi |
| `EntityId id` | Truyền một số theo giá trị |
| `std::string title` | Nhận chuỗi theo giá trị, khác với `const std::string&` |
| `EntityId id() const` | Trả bản sao ID; không sửa các thành viên thông thường của đối tượng qua `this` |
| `const std::string&` | Trả tham chiếu chỉ đọc đến chuỗi đang nằm trong entity |
| `private:` | Code bên ngoài không trực tiếp truy cập các trường này |
| `Film::` trong `.cpp` | Hàm thuộc phạm vi lớp Film; không phải namespace tên Film |
| `this->title_` | Truy cập thành viên của chính đối tượng hiện tại |

`const` sau tên hàm không đóng băng mọi đối tượng có liên quan. Một thành viên
tham chiếu/con trỏ vẫn có thể dẫn đến dữ liệu sửa được nếu kiểu đích không const.
`struct` và `class` đều có thể có constructor/phương thức; khác mặc định về
quyền truy cập và kiểu kế thừa. Các struct kết quả ở đây chọn dữ liệu public.

Trong constructor:

```cpp
this->title_ = entity_detail::name(title);
```

Gọi hàm kiểm tra/chuẩn hóa trước, rồi gán kết quả. Nếu tên sai, constructor
không hoàn tất; các thành viên đã khởi tạo được dọn tự động. `std::string`
đã được dựng trước khi vào thân constructor; đây là phép gán, không phải
danh sách khởi tạo thành viên. Biến số được gán trước khi sử dụng.

`name()` chỉ cắt dấu cách ASCII đầu/cuối; từ chối ký tự điều khiển và `|`.
Nó không tự kiểm tra toàn bộ UTF-8; bộ đọc/ghi TXT và bước validate làm việc đó.
`dateTime()` kiểm tra chiều dài, vị trí dấu, chữ số rồi ngày lịch/năm nhuận.
Phải kiểm tra tháng trước khi truy cập `daysPerMonth[month - 1]`.

Với `Auditorium`, kiểm tra `rows > maximum / columns` trước khi nhân.
Nếu nhân trước rồi mới kiểm tra, phép nhân số nguyên có dấu đã có thể tràn.
`capacity()` chỉ nhân hai giá trị đã được constructor kiểm tra.

Tự kiểm tra: vì sao `Seat(1, 999, 1, 1)` có thể tạo được dù phòng 999 không tồn tại?
Đáp án: constructor chỉ biết giá trị của ghế; service/validate mới có repository phòng.

## 4. DSA: biết từng con trỏ trỏ vào đâu

### LinkedList

`Node` chứa `T value`, `previous`, `next`. `head_`, `tail_` và `size_` thuộc list.
Node giữ bản sao T, không phải tham chiếu tới biến được truyền vào.

```cpp
Node* node = new Node{value, tail_, nullptr};
```

`new` cấp phát và khởi tạo node: copy `value`, trỏ về đuôi cũ, chưa có node sau.
Nếu cấp phát/copy thất bại, câu lệnh không trả node; list chưa được nối lại.
Sau đó `pushBack` cập nhật đuôi cũ hoặc đầu list rỗng, gán đuôi mới, tăng số lượng.
Giá trị trả `T&` là entity nằm trong node mới, không phải biến đầu vào.

`erase(pointer)` duyệt tìm **đúng địa chỉ**, không tìm entity có cùng ID.
`eraseNode` nối hai phía, sửa head/tail nếu cần, rồi delete và giảm size.
`clear()` lưu `next` trước khi delete node hiện tại để không đọc vùng nhớ đã hủy.
Hãy tự vẽ bốn trường hợp xóa: đầu, giữa, cuối, node duy nhất.

`find(value)` cần toán tử `==` của T khi được sử dụng. Các entity hiện không
cung cấp toán tử đó; repository không gọi `LinkedList<Film>::find`.
Template không bắt mọi kiểu phải hỗ trợ mọi hàm chưa được dùng.

### Iterator không phải bản sao danh sách

Iterator chỉ giữ con trỏ node hiện tại. `*it` lấy giá trị, `++it` đi sang node kế,
`it != end` kiểm tra hết. `end()` chứa `nullptr`, không được giải tham chiếu.

Minh họa cách đọc vòng lặp, không phải đoạn thay thế bắt buộc:

```cpp
for (const Film& film : films) {
    // Doc film.
}
// Co the hieu tuong duong:
for (LinkedList<Film>::ConstIterator it = films.begin(); it != films.end(); ++it) {
    const Film& film = *it;
    // Doc film.
}
```

Ở minh họa này `films` là một `FilmRepository` hoặc danh sách const.
Danh sách không const trả `Iterator` có thể sửa; repository luôn trả
`ConstIterator`. `const` trên `operator*() const` cấm sửa iterator qua `this`,
không tự làm giá trị trả về bất biến: phải nhìn cả `T&` hay `const T&`.

### HashTable

`Hasher` là kiểu có `operator()`; `hasher_(key) % bucketCount_` chọn bucket.
`DefaultHash<std::string>` là chuyên biệt hóa template cho chuỗi. Tràn của biến
hash không dấu là phép tính modulo miền số không dấu; khác tràn số có dấu.
Va chạm không có nghĩa hai khóa bằng nhau: phải duyệt bucket và so sánh `entry.key`.

`Value* find(...)` trả địa chỉ ô giá trị trong entry. Khi Value là `Film*`,
kết quả thường là `Film**`, không phải một Film mới.

`insert` copy `candidate` trước rehash để trường hợp đầu vào nằm trong bảng cũ
không bị mất hiệu lực trước khi copy. `rehash` cấp phát bucket mới, sao chép
entries, chỉ thay bảng cũ sau khi thành công. Catch xóa bảng tạm rồi `throw;`.
Con trỏ tới entry cũ mất hiệu lực khi rehash; con trỏ entity chứa trong entry
vẫn trỏ tới node của repository như trước.

`erase` xóa entry rồi return ngay, không tăng iterator đã trỏ vào node bị xóa.
`delete[] buckets_` gọi destructor từng bucket; nếu Value là con trỏ entity,
hủy giá trị con trỏ không đồng nghĩa `delete` entity.

### Matrix2D

Mảng một chiều `new T[rows * columns]{}` lưu các ô; `{}` khởi tạo giá trị.
`at(row, column)` kiểm tra biên rồi dùng `row * columns_ + column`.
Hàng/cột trong entity bắt đầu từ 1; chỉ số ma trận từ 0, nên dựng sơ đồ phải trừ 1.
Destructor dùng `delete[]`, không dùng `delete`.

Tự kiểm tra: tại sao list cấp phát `new Node` còn bảng bucket dùng `new Bucket[count]`?
Đáp án phải kèm cặp delete tương ứng và ai chịu trách nhiệm gọi.

## 5. Repository: đọc những dòng khó

### Con trỏ hai cấp

Đoạn thật từ `findById`, nằm trong hàm const:

```cpp
T* const* indexed = this->index_.find(id);
if (indexed == nullptr)
{
    return nullptr;
}
const T* record = *indexed;
return record;
```

1. Giá trị của bảng băm là `T*`. Bản `find() const` trả `const Value*`.
2. Thay Value bằng `T*` được `T* const*`: con trỏ tới một ô con trỏ không được sửa qua đường này.
3. `indexed == nullptr` nghĩa không có entry; chưa được dùng `*indexed`.
4. `*indexed` lấy địa chỉ entity; gán sang `const T*` để người gọi chỉ đọc entity.
5. `return record` không copy entity. Địa chỉ này mất hiệu lực khi bản ghi bị xóa/thay bộ.

Hai const có vị trí khác nhau không tương đương:
`T* const*` bảo vệ ô con trỏ; `const T*` bảo vệ đối tượng được trỏ tới qua con trỏ đó.

### Bất biến và rollback

`insert` phải thêm thành công cả list và index. Catch gỡ node mới rồi ném lại
đúng exception; dirty và highestId chỉ đổi sau khi cả hai hoàn tất.
`nextId()` chưa tăng ID; không nên nói gọi hai lần sẽ có hai ID khác nhau.

`update` dùng:

```cpp
T replacement(value);
using std::swap;
swap(*record, replacement);
```

Copy có thể lỗi; nếu lỗi thì record chưa đổi. Swap đổi giá trị trong chính node,
địa chỉ node vẫn giữ nguyên; replacement nhận giá trị cũ và dọn nó khi ra khỏi scope.
`static_assert(std::is_nothrow_swappable_v<T>, ...)` kiểm tra lúc biên dịch rằng
bước swap không ném lỗi. `using std::swap` đưa hàm chuẩn vào phạm vi tìm tên,
đồng thời cho phép tìm overload phù hợp; không phải `using namespace std`.

`replaceWith` phải đổi cả list/index. Nếu chỉ đổi list, index sẽ trỏ sai bộ dữ liệu.
`dirty_` không phải cờ “đã bán vé”; nó là trạng thái repository đã có thao tác thay đổi.

### Năm hàm đặc biệt

| Khai báo | Nghĩa |
|---|---|
| `Repository() = default` | Yêu cầu compiler sinh constructor mặc định |
| `Repository(const Repository&) = delete` | Cấm tạo bản sao |
| `operator=(const Repository&) = delete` | Cấm gán sao chép |
| `Repository(Repository&&) = delete` | Cấm tạo bằng chuyển quyền sở hữu |
| `operator=(Repository&&) = delete` | Cấm gán chuyển quyền sở hữu |

`= delete` không phải lệnh giải phóng bộ nhớ. `&&` ở chữ ký này là tham chiếu
rvalue, không phải toán tử AND. Không gọi tất cả chúng là “đa hình”.
Muốn cho copy phải xử lý đúng list, index, quyền sở hữu và lỗi cấp phát; không chỉ bỏ `delete`.

`typename LinkedList<T>::ConstIterator` báo tên phụ thuộc T này là một kiểu.
`using value_type = T` giúp hàm TXT tìm ra kiểu entity của repository.
Template để định nghĩa trong header cho nơi sử dụng nhìn thấy mã khi khởi tạo
kiểu cụ thể; khác với hàm thường đã được liên kết từ `.cpp`.

Trong `HashTable::swap`, `noexcept(std::is_nothrow_swappable_v<Hasher>)` là
cam kết có điều kiện theo kiểu Hasher; không phải lời gọi hàm kiểm tra lúc chạy.
`friend class ApplicationData` cấp quyền cho cả lớp, không chỉ riêng `load()`;
quyền này phải được dùng có giới hạn trong code điều phối.

Tự kiểm tra: tại sao `TicketRepository` có hàm giống repository nhưng không
phải override? Vì nó không kế thừa repository và không có hàm virtual tương ứng;
nó gọi chuyển tiếp sang thành viên bên trong.

## 6. Service: ba luồng phải tự lần được

### Tạo hoặc sửa phòng

`CatalogService::prepareSeats` chép ghế của các phòng khác vào repository tạm,
bỏ bộ ghế của phòng đang thay; nếu có cấu hình mới thì sinh ghế theo offset:
`row = offset / columns + 1`, `column = offset % columns + 1`.
Kiểm tra đủ miền ID trước khi cộng. Sau khi chuẩn bị thành công mới đổi phòng
và thay bộ ghế bằng `replaceWith`.

Không phải sửa từng ghế trực tiếp trong repository chính. Lỗi chuẩn bị thì
phòng/ghế cũ còn nguyên; thành công thì địa chỉ ghế cũ không còn dùng lâu dài được.
`makeSeatMap` kiểm tra trùng vị trí, ngoài phòng, thiếu ghế; `unique_ptr` dọn
ma trận tạm nếu bất kỳ bước nào ném lỗi.

### Tạo suất

`ScreeningService::add` tạo candidate rồi validate trước insert.
`service_validation::screening` tra phim/phòng, quy đổi giờ ra phút, bỏ qua chính
ID đang sửa, chỉ đối chiếu các suất cùng phòng.

Hai khoảng `[a, b)` và `[c, d)` trùng khi `a < d && c < b`.
`b`, `d` gồm 15 phút nghỉ; bắt đầu đúng lúc kết thúc khoảng kia được phép.
Ví dụ phim 120 phút bắt đầu 18:00 giữ phòng đến 20:15; suất 20:14 bị từ chối,
20:15 được nhận nếu các điều kiện khác đúng.

`screening_time::minutes` trong `ScreeningTime.cpp` cộng số ngày
của các năm trước, tháng trước và ngày hiện tại, rồi nhân 1440, cộng giờ/phút.
`formatMinute` làm ngược lại. Không phải phép trừ trực tiếp hai chuỗi ngày.
So sánh chuỗi dùng được cho thứ tự thời gian vì định dạng cố định và đã kiểm tra;
không đúng nếu dùng chuỗi như `2/11/2026`.
Code không xử lý timezone/DST; `formatMinute` được gọi với phút trong miền hợp lệ.

`ScreeningService::validate` thêm điều kiện tương lai.
`ApplicationData::validate` không thêm điều kiện này để còn nạp lịch sử.

### Bán vé: giải thích từng bước quan trọng

Trong `TicketService::sell`:

| Dòng/thao tác | Ý nghĩa |
|---|---|
| `if (!cashConfirmed) throw ...` | Không tạo vé khi chưa xác nhận nhận tiền |
| `const SaleDetails sale = details(...)` | Tra ID, đúng phòng; tạo bản sao dữ liệu dùng cho lần bán |
| `const std::string now = clock_();` | Gọi con trỏ hàm; lấy thời điểm cho cả kiểm tra và ghi vé |
| `requireAvailable(sale, now);` | Kiểm tra chưa bắt đầu và chưa có vé cùng suất-ghế |
| `const EntityId id = tickets_.nextId();` | Lấy mã dự kiến, có kiểm tra tràn |
| `const std::int64_t price = sale.screening.ticketPrice();` | Giá từ suất, không tin giá nhập từ UI |
| `const Ticket ticket(id, ..., now, price);` | Tạo entity hợp lệ trước khi đưa vào repository |
| `tickets_.insert(ticket);` | Copy vào RAM và index; chưa ghi TXT |
| `return id;` | UI nhận mã vé để cập nhật hiển thị |

`insert` trả bool nhưng service không kiểm tra lại vì `nextId` lớn hơn mọi ID
đã nhận và chỉ hỗ trợ một phiên, không sửa xen kẽ. Đây là giả định của luồng
hiện tại, không phải bảo đảm an toàn đa luồng.

`preview()` không giữ chỗ, không tạo vé, không tăng doanh thu. Vì người dùng
có thể chờ ở hộp xác nhận, `sell()` phải kiểm tra lại giờ và ghế.
Kiểm tra lại không thay thế khóa/transaction trong hệ thống nhiều phiên.

`revenue` cộng giá đã chốt trên vé; kiểm tra tràn trước cộng.
`listBySaleDate` lọc cùng ngày và gọi `find` để dựng thông tin chi tiết.
Vé vừa bán xuất hiện ngay vì truy vấn đọc RAM, không đọc lại TXT.

## 7. TXT và ApplicationData

Đọc thêm [quy trình lưu](design/txt-format.md); cần tự theo dõi các cờ
`ready_`, `hadFiles_`, `ownsStage_`, `recoveryRequired_`, không học chung là “cờ trạng thái”.

Trong template `load`, `using T = typename RepositoryType::value_type` lấy kiểu
bản ghi. `const T* tag = nullptr` chỉ dùng kiểu con trỏ để chọn overload
`header(tag)`/`parse(fields, tag)`; các hàm đó không dereference tag.
Không có đối tượng được tạo ở địa chỉ null, không có virtual dispatch.

`from_chars` phải kiểm tra cả mã lỗi và đã đọc hết trường; `120abc` không phải 120.
Trong kiểm tra số trường, biểu thức:

```cpp
(i + 1 == fieldCount) != (end == std::string::npos)
```

So sánh hai bool: “đang ở trường cuối” và “không còn dấu phân cách”.
Hai điều này phải cùng đúng hoặc cùng sai; khác nhau nghĩa thiếu/thừa trường.
`npos` là dấu hiệu không tìm thấy, không phải chỉ số hợp lệ để truy cập chuỗi.

UTF-8: byte đầu xác định số byte nối tiếp; `(byte & 0xc0) == 0x80` kiểm tra
tiền tố byte nối; dịch trái 6 bit để ghép phần dữ liệu. Các kiểm tra cuối từ chối
mã hóa dài hơn cần thiết, surrogate và mã vượt giới hạn. Không được xóa các
kiểm tra chỉ vì tiếng Việt demo vẫn hiển thị đúng.

`load()` của bộ codec có thể đã thêm một số dòng trước khi lỗi.
Tính “không nạp một phần vào dữ liệu chính” đến từ `ApplicationData staged`:
đọc đủ bộ, validate, rồi mới swap sang dữ liệu chính.

`validate()` tạo lại entity tạm để chạy constructor kiểm tra giá trị; không thêm
bản ghi. Ba hash tạm kiểm tra vị trí ghế, đếm ghế/phòng và trùng suất-ghế.
`path`/`line` giúp gắn vị trí đang xét vào lỗi; lỗi có thể được phát hiện trong khi
đối chiếu một bản ghi khác. Không khẳng định số dòng luôn chỉ nguyên nhân duy nhất.

`save()` kiểm tra file, ghi toàn bộ new, sao lưu old rồi thay từng file chính.
`installed` đếm số file đã thay để biết phạm vi rollback khi bắt lỗi.
`std::current_exception` giữ lỗi gốc trong lúc thử phục hồi; `rethrow_exception`
đưa lỗi đó trở lại để báo cho UI. `catch (...)` bắt cả lỗi không thuộc
`std::exception`; `throw;` ném lại lỗi đang xử lý.

`isDirty == false` không làm `save()` bỏ qua ghi; chương trình vẫn xuất cả bộ.
`flush`/`close` kiểm tra lỗi stream, không tương đương cam kết fsync khi mất điện.
Các giới hạn này khác transaction DBMS, xem phần ACID trong thiết kế dữ liệu.

## 8. Qt, main và hệ thống build

`main.cpp`: tạo `QApplication`, đọc đường dẫn, tạo `ApplicationData`, nạp thành
công rồi mới tạo ba service và cửa sổ. `PBL2_DEMO_DATA_DIR` được CMake định nghĩa
bằng đường dẫn data/demo lúc build; không phải lúc chạy tự suy ra từ thư mục hiện tại.
`app.exec()` chạy vòng lặp sự kiện; không phải chạy lại hàm main liên tục.

| Nhóm hàm trong MainWindow | Đọc và giải thích |
|---|---|
| `setup...Tab` | Tạo widget, bố trí layout, đặt objectName và nối sự kiện cho năm tab |
| `selected...`, `comboId` | Lấy ID, không lấy số thứ tự dòng làm khóa nghiệp vụ |
| `refresh...`, `populateFilms/Rooms` | Gọi service, đổ dữ liệu, khôi phục lựa chọn; chặn signal khi cập nhật |
| `edit...`, `delete...` | Dialog/xác nhận → gọi service → refresh → trạng thái chưa lưu |
| `selectSaleSeat`, `sellTicket` | Xem trước và xác nhận; một lần bán một ghế |
| `lookupTicket`, `showTicketDetails`, `refreshRevenue` | Tìm mã → chọn ngày bán → danh sách/chi tiết; không phải hai nguồn vé khác nhau |
| `closeEvent` | Ignore trước, xác nhận, thử phục hồi/lưu, chỉ accept khi thành công |
| `fromUtf8`, `toUtf8`, `money`, `seatName` | Chuyển đổi/định dạng hiển thị, không thay dữ liệu gốc |
| `positiveInput`, `positiveValue`, `reportError`, `errorText`, `confirm` | Giới hạn nhập, kiểm tra chuyển số, thông báo và xác nhận |
| `configureTable`, `setNumber`, `MoneyItem` | Cấu hình bảng; giữ giá trị số để sắp xếp không theo chuỗi tiền |

`FilmDialog`, `RoomDialog`, `ScreeningDialog` kế thừa `QDialog`, nằm trong
namespace vô danh của `.cpp`, chỉ phục vụ cửa sổ này; không phải entity.

Ví dụ signal thật:

```cpp
connect(filmAdd, &QPushButton::clicked, this, [this] { editFilm(true); });
```

Sender là nút, `&QPushButton::clicked` chỉ signal, `this` là đối tượng ngữ cảnh.
`[this]` giữ con trỏ cửa sổ để gọi `editFilm`; không sao chép cả cửa sổ.
Code chạy khi có signal, không chạy ngay tại dòng connect.
Trong dialog, lambda có thể giữ tham chiếu service; service phải còn sống.
Qt quản lý việc ngắt kết nối theo vòng đời đối tượng ngữ cảnh.

`QSignalBlocker` chặn signal tạm thời và khôi phục khi ra khỏi scope, kể cả có
exception. Các bảng tắt sorting khi điền nhiều cột để dòng không bị đổi vị trí
giữa chừng, rồi bật lại. `Qt::UserRole` giữ ID/trạng thái riêng với text hiển thị.
`item->flags() & ~Qt::ItemIsSelectable` bỏ một bit cờ, không phải phủ định bool.

`override` yêu cầu compiler kiểm tra chữ ký ghi đè đúng hàm virtual của lớp cha.
`MoneyItem::operator<` là ví dụ vừa dùng cú pháp toán tử vừa ghi đè hàm virtual;
khác `Iterator::operator++` không virtual.

Widget có parent thường được parent dọn; item được `setItem` giao cho bảng.
Dialog cục bộ sống trên stack và tự hủy. Con trỏ tới service không được cửa sổ
delete. Hãy chỉ ra người sở hữu từng `new`, không trả lời chung “Qt lo hết”.

Trong CMake: `.cpp` được biên dịch vào thư viện/executable; include chỉ cung cấp
khai báo, không tự liên kết implementation. `target_link_libraries` nối thư viện
core/UI/Qt. Nếu tách hàm thường sang `.cpp` mà quên thêm vào target, có thể gặp
`undefined reference`. Core không cần Qt; `BUILD_GUI=OFF` chỉ tạo bảy test lõi.

## 9. Những từ khóa còn lại không được né

| Cú pháp | Nghĩa trong dự án |
|---|---|
| `#pragma once` | Tránh xử lý lặp nội dung header trong cùng translation unit; không tự giải quyết mọi lỗi nhiều định nghĩa |
| Forward declaration `class CatalogService;` | Cho phép khai báo con trỏ/tham chiếu khi chưa cần định nghĩa đầy đủ |
| `explicit` | Chặn chuyển đổi ngầm qua constructor một đối số |
| `final` | Cấm kế thừa tiếp lớp; không có nghĩa mọi phương thức đều virtual |
| `static` trên hàm | Không có `this`, không cần đối tượng để gọi; vẫn chịu quyền truy cập |
| `static const` cục bộ | Giá trị dùng chung giữa các lần gọi; ví dụ phút lớn nhất được tính một lần |
| `static_cast` | Chuyển kiểu tường minh, không tự kiểm tra mọi trường hợp tràn |
| `enum class LoadResult` | Kiểu liệt kê có phạm vi; dùng `LoadResult::Loaded` |
| `friend` | Cấp quyền truy cập private/protected cho đúng lớp/hàm được chỉ định; không phải kế thừa |
| `noexcept` | Cam kết không ném lỗi ra ngoài; nếu vi phạm sẽ terminate, không có nghĩa tự bắt lỗi |
| `std::make_unique` | Tạo đối tượng và chuyển quyền quản lý cho một unique_ptr |
| `std::size_t` | Kiểu số không dấu dùng kích thước/chỉ số; cần tránh trừ gây cuộn về số lớn |
| `std::numeric_limits<T>::max()` | Giá trị lớn nhất của kiểu, dùng kiểm tra trước khi cộng/nhân |
| `?:` | Chọn một trong hai biểu thức; không phải chạy cả hai nhánh |

Template/overload là cơ chế ở thời điểm biên dịch; hàm virtual là cơ chế chọn
implementation theo đối tượng lúc chạy. Đừng gọi mọi cú pháp lạ là “đa hình”.

## 10. Bộ câu hỏi tự vấn đáp

Tự trả lời trước; các ý sau là điểm đối chiếu, không phải bài phát biểu học thuộc.

| Câu hỏi | Ý bắt buộc |
|---|---|
| Bài có bốn tính chất OOP ở đâu? | Chỉ đúng code đóng gói/trừu tượng/kế thừa; đa hình động có ở Qt, không tự nhận core dùng hàm ảo |
| Có phải FilmRepository kế thừa Film? | Không, kế thừa Repository có tham số kiểu Film |
| Vì sao TicketRepository chứa thay vì kế thừa? | Hạn chế public API sửa/xóa; đổi lại phải viết hàm chuyển tiếp |
| Vì sao không lưu con trỏ Film trong Screening? | ID phù hợp TXT, tra lại qua repository, không phụ thuộc địa chỉ RAM |
| Dữ liệu bị copy ở đâu? | Entity vào node; kết quả truy vấn/chi tiết; service giữ reference, không copy repository |
| Xóa phim có cần xóa suất theo không? | Hiện chặn xóa phim có suất, không cascade |
| Chỉ có PK ticket_id đã chống bán trùng chưa? | Chưa, phải kiểm tra khóa nghiệp vụ suất-ghế |
| Vì sao không có isSold trong Seat? | Trạng thái tùy suất, suy ra từ Ticket |
| Hai FK hợp lệ trên vé đã đủ chưa? | Chưa, ghế còn phải thuộc cùng phòng với suất |
| Chứng minh 3NF thế nào? | Nêu FD/khóa ứng viên, kiểm tra phụ thuộc; không dùng “có ID” làm chứng minh |
| unit_price có phụ thuộc screening_id không? | Không được áp đặt trên toàn dữ liệu được nạp; giải thích snapshot và quy tắc giá lịch sử |
| Tìm ID có luôn O(1)? Xóa có O(1)? | Hash trung bình O(1), xấu nhất tuyến tính; xóa repository duyệt list O(n) |
| Gọi markSaved có lưu file không? | Không, chỉ xóa cờ; điều phối gọi sau khi lưu đủ bộ |
| Bán thành công rồi tắt nguồn thì sao? | Vé mới chỉ trên RAM, có thể mất trước khi lưu |
| Hai cửa sổ/tiến trình có chống bán trùng không? | Không có cơ chế phối hợp đa phiên |
| Có thể bỏ service và gọi repository? | Code có thể làm được nhưng bỏ qua nghiệp vụ; đây là giới hạn kiến trúc hiện tại |
| Test pass có chứng minh không rò bộ nhớ? | Không; assert/sanitizer chỉ quan sát phạm vi chạy, UBSan không thay công cụ dò leak |

## 11. Thực hành để biết mình thực sự hiểu

Làm trên bản sao dữ liệu, không thử phá file demo dùng nộp bài.

1. Vẽ list/index cho hai phim; mô phỏng thêm, update, rehash và xóa. Chỉ rõ địa
   chỉ nào giữ nguyên, địa chỉ nào không còn dùng được.
2. Tạo phòng 2 × 3, tự tính sáu hàng/cột từ offset rồi so với code.
3. Kiểm tra hai suất đúng biên 15 phút; giải thích từng nhánh `continue`.
4. Bán một ghế, hủy lần xác nhận tiếp theo; chỉ ra số vé, dirty, sơ đồ và file đổi ở đâu.
5. Trên bộ TXT sao chép, tạo hai ticket khác ID nhưng cùng suất-ghế; chương trình
   phải từ chối nạp. Chỉ được file/hàm báo lỗi và vì sao UI chưa được mở.
6. Thử ngày bán không có vé; giải thích vì sao trả 0 dù không có một dòng “báo cáo” trong TXT.
7. Bài sửa nhỏ: thêm bộ lọc giá tối đa cho suất. Trước khi sửa, xác định chữ ký
   service, điều kiện lọc, input Qt và test; không sửa entity/TXT nếu không lưu thuộc tính mới.

Đọc `tests/RepositoryTest.cpp` để thấy cách ép copy/cấp phát lỗi; đọc
`tests/ApplicationDataTest.cpp` để thấy lỗi ghi, rename và rollback.
`operator new` thay thế trong test và callback `beforeFileStep_` là công cụ
giả lập lỗi, không phải hành vi bình thường của ứng dụng.
Các test bỏ `NDEBUG` trước include assert để bản Release vẫn kiểm tra điều kiện.

Chạy tại thư mục dự án:

```bash
ctest --test-dir build --output-on-failure
ctest --test-dir build -R 'Repository|Service|ApplicationData' --output-on-failure
```

Đủ sẵn sàng khi tự giải thích được một luồng, dự đoán trường hợp lỗi và sửa một
yêu cầu nhỏ kèm test; không chỉ nhớ tên lớp hoặc học thuộc sơ đồ.
