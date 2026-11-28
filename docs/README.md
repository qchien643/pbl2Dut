# Tài liệu đồ án PBL2

Cập nhật ngày **28/11/2026**. Mỗi chủ đề có một tài liệu chịu trách nhiệm chính;
tài liệu khác dẫn liên kết hoặc mã FR/NFR/BR, không chép lại toàn bộ nội dung.

## 1. Cấu trúc

```text
docs/
├── README.md                     Điểm bắt đầu và trách nhiệm tài liệu
├── requirements/                 Sản phẩm cần làm gì; chính sách rạp
├── design/                       Dữ liệu, OOP, service và đọc/ghi TXT
├── diagrams/                     Mỗi sơ đồ: README.md + HTML cùng thư mục
├── guides/                       Sử dụng, vấn đáp và ảnh Qt trong images/
└── testing/                      Kiểm thử và giới hạn bằng chứng
```

## 2. Trách nhiệm từng tài liệu

| Tài liệu | Chịu trách nhiệm | Không chứa |
|---|---|---|
| [README dự án](../README.md) | Cài đặt, build, chạy, cấu trúc thư mục | Đặc tả chi tiết, giảng từng hàm |
| [SRS](requirements/srs.md) | Phạm vi, chức năng FR và chất lượng NFR cần đáp ứng | Thuật toán, bảng cột TXT, danh sách hàm |
| [Business Rules Catalog](requirements/business-rules.md) | Chính sách của rạp, điều kiện cho phép/cấm, mã BR | Dependency, repository, rollback, test |
| [Thiết kế dữ liệu](design/data-design.md) | Lược đồ, thuộc tính, PK/FK, lực lượng quan hệ, chuẩn hóa | Cách parse/lưu file, luồng gọi hàm |
| [Thiết kế lớp](design/class-design.md) | Phân tầng, sở hữu, association, kế thừa, DI/DIP | Danh mục chính sách hoặc từng hàm service |
| [Thiết kế service](design/service-design.md) | Hành vi hàm, luồng phối hợp và ánh xạ BR sang code | Định nghĩa lại chính sách của rạp |
| [Thiết kế TXT](design/txt-format.md) | Định dạng file, đọc/ghi, kiểm tra nạp/lưu, phục hồi | Quy trình thao tác giao diện |
| [Feature map giao diện](design/ui-feature-map.md) | Từng tính năng theo tab, đầu vào, cách tìm/lọc, kết quả, giới hạn và điểm thảo luận | Tự đặt chính sách rạp hoặc coi đề xuất là chức năng đã có |
| [Thiết kế index tên phim](design/film-name-search.md) | Xây và đồng bộ index trigram trên RAM, cách phục vụ tìm tên và ghép vào code | Định nghĩa thay SRS hoặc chứa số đo hiệu năng |
| [Hướng dẫn sử dụng](guides/user-guide.md) | Các bước vận hành năm tab, lưu/thoát và lỗi thường gặp | Giải thích OOP hoặc implementation |
| [Kiểm thử](testing/verification.md) | Lệnh chạy, đối chiếu FR/NFR/BR với test, giới hạn bằng chứng | Tự đặt yêu cầu/nghiệp vụ mới |
| [Benchmark tìm tên](testing/film-search-benchmark.md) | Dữ liệu, cách đo, kết quả index so với scan, CSV và giới hạn | Khẳng định index luôn nhanh hơn hoặc thay thế SRS |
| [Bộ bảy sơ đồ](diagrams/README.md) | Minh họa kiến trúc, OOP, ERD và ba luồng chính; ghi nguồn/phạm vi | Bản sao toàn bộ yêu cầu và BR |
| [Hướng dẫn vấn đáp](guides/study-guide.md) | Lộ trình học, câu hỏi và bài tập lần theo code | Bản sao đầy đủ của các tài liệu thiết kế |

## 3. Cách đọc

- Hiểu sản phẩm: SRS → Business Rules → feature map → hướng dẫn sử dụng.
- Hiểu cài đặt: thiết kế dữ liệu → lớp → service → TXT.
- Tìm tên phim: feature map tab Phim → [thiết kế index](design/film-name-search.md) → [benchmark](testing/film-search-benchmark.md).
- Kiểm chứng: kiểm thử; ôn môn học: hướng dẫn vấn đáp.

Ví dụ: quy tắc không trùng tên ở BR-C04/C05; cách hàm kiểm tra ở thiết kế
service; ca kiểm chứng ở tài liệu kiểm thử. Chỉ sửa định nghĩa chính sách tại BR.

## 4. Quy tắc cập nhật

- FR/NFR chỉ định nghĩa ở SRS; BR chỉ định nghĩa ở Business Rules. Tài liệu
  thiết kế giải thích cách thực hiện và dẫn mã, không đặt lại chính sách.
- Khi code đổi, cập nhật tài liệu chủ đề và cặp mô tả/hình liên quan. Mỗi mô tả
  sơ đồ ghi mục đích, cách đọc, nguồn code, giới hạn và ngày đối chiếu.
- Không giữ nhiều bản thiết kế mâu thuẫn trong docs. Sơ đồ Archify cũ, file
  sinh kèm và wireframe dữ liệu giả đã bỏ; lịch sử vẫn nằm trong Git.
- Kiểm tra trước khi bàn giao: `python3 tests/check_docs.py`. Script không
  thay thế đối chiếu ý nghĩa với source hoặc review hình trên trình duyệt.

Các tài liệu hiện trạng phản ánh source; index tên
được triển khai và kiểm thử ngày 28/11/2026. Sơ đồ đối chiếu
ngày 27/11/2026. `slides/` và `ref/oop/` là tài liệu môn học ở máy phát triển,
đang được Git bỏ qua, không thuộc bộ tài liệu sản phẩm bàn giao.
