# Sơ đồ tương tác bằng Archify

Đối chiếu mã nguồn ngày **26/10/2026**. Mở trực tiếp HTML bằng trình duyệt,
không cần chạy Qt, server hay cài Archify để xem.

| Sơ đồ | Nội dung ôn tập |
|---|---|
| [Kiến trúc ứng dụng](architecture/index.html) | Qt → service → RAM; nhánh lưu qua ApplicationData và TXT |
| [Quan hệ lớp OOP](classes/index.html) | Kế thừa lớp mẫu, composition và tham chiếu; chỉ chọn một lát cắt repository |
| [ERD năm thực thể](erd/index.html) | Đầy đủ cột TXT, PK/FK, lực lượng quan hệ và ràng buộc ghép |
| [Luồng hoạt động](workflow/index.html) | Khởi động, nạp TXT, thao tác trên RAM, lưu an toàn và xử lý lỗi |

Nội dung bằng tiếng Việt; các nút của trình xem và thuộc tính HTML `lang`
dùng tiếng Anh do phiên bản này chưa có bản dịch `vi`. Chọn một nút để xem
tham chiếu mã nguồn. Đây là tài liệu mô tả, không thay đổi kiến trúc chương trình.
ERD là mô hình dữ liệu TXT, không khẳng định dự án đã dùng SQL.

## Công cụ đã cài

- Nguồn: `https://github.com/tt-a1i/archify.git`
- Archify 3.2.0, commit `81c6be8d87090670c45d08a8f97f9d29c5af0bec`.
- Đã dùng `git clone`, rồi cài thư mục skill `archify` bằng trình cài skill
  của Codex vào `/home/luulomhe/.codex/skills/archify`.
- Khởi động lại phiên Codex để skill mới được tự nhận diện.
  Lần tạo này đã đọc và thực hiện hướng dẫn skill trực tiếp.
- Công cụ không phải dependency của ứng dụng C++; không thêm nó vào CMake.

## Nguồn kiểm chứng

Thư mục PBL2 tại thời điểm chạy không có lịch sử Git. Vì Archify kiểm chứng
nguồn tại một commit, đã tạo **bản chụp Git riêng** trong
`.cache/archify-source-KE20RS`, chỉ gồm `src/` và `CMakeLists.txt`.
Không khởi tạo Git ở gốc dự án, không commit dự án và không push.

Commit bản chụp: `103602a1e84c4b81641c6b5d1ea0ae56e1195f91`.
Bản chụp được đóng gói trong `source.bundle` để có thể kiểm chứng lại trên máy khác.
URL GitHub trong metadata chỉ nhận diện dự án; `link_mode: local-only`
không tạo liên kết đến một commit chưa được công bố.

Mỗi thư mục sơ đồ chứa `candidate.json`, HTML và biên bản do Archify sinh.
Các bước validate, deliver, check và browser-check đã qua; đây là kiểm tra
tự động, chưa phải đánh giá thẩm mỹ bằng mắt.

Sơ đồ workflow đối chiếu trực tiếp với commit mã nguồn `1248350` đã công bố
trên GitHub; các sơ đồ cũ dùng bản chụp nguồn cục bộ được mô tả ở trên.

## Tạo lại

Cần Node.js >= 18, Chrome/Chromium và skill Archify đã cài. Chạy từ gốc PBL2:

```bash
evidence_dir=$(mktemp -d)
git clone docs/diagrams/source.bundle "$evidence_dir"
git -C "$evidence_dir" remote set-url origin https://github.com/qchien643/pbl2Dut.git
archify_cli="$HOME/.codex/skills/archify/bin/archify.mjs"
review_dir=$(mktemp -d)
node "$archify_cli" finalize architecture docs/diagrams/architecture/candidate.json docs/diagrams/architecture/index.html --repo-root "$evidence_dir" --quality showcase --out-dir "$review_dir/architecture" --json
node "$archify_cli" finalize class docs/diagrams/classes/candidate.json docs/diagrams/classes/index.html --repo-root "$evidence_dir" --quality showcase --out-dir "$review_dir/classes" --json
node "$archify_cli" finalize erd docs/diagrams/erd/candidate.json docs/diagrams/erd/index.html --repo-root "$evidence_dir" --quality showcase --out-dir "$review_dir/erd" --json
```

Các lệnh này tái tạo từ **bản chụp cũ**, không tự cập nhật theo code mới.
Khi code thay đổi, phải đối chiếu lại nội dung, dòng nguồn và revision trong
`candidate.json` trước khi xuất lại.
