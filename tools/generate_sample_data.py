"""Generate a new, deterministic sample directory; never overwrite existing data."""

import argparse
import csv
from datetime import datetime, timedelta
import json
from pathlib import Path


def generate(destination, film_count=1000):
    if film_count < 1000:
        raise ValueError("Use at least 1000 films")
    titles = [
        "Nguyễn Trãi", "Người Nhện", "Mắt Biếc", "Chuyến Tàu Hoàng Hôn",
        "Dòng Sông Ký Ức", "Ánh Sáng Cuối Trời", "Thành Phố Trong Mưa",
        "Bí Mật Đại Dương", "Hành Trình Phương Bắc", "Đường Về Quê Hương",
        "Avatar", "Titanic", "Interstellar", "Inception", "The Dark Knight",
        "Những Ngày Không Quên", "Chiến Binh Rồng Lửa", "Vườn Hoa Mùa Xuân",
        "Gia Đình Hạnh Phúc", "Bầu Trời Tuổi Thơ",
    ]
    modifiers = ["Bình Minh", "Hoàng Hôn", "Mùa Đông", "Ký Ức", "Tương Lai",
                 "Huyền Thoại", "Phiêu Lưu", "Tình Yêu", "Giấc Mơ", "Vũ Trụ"]
    films = []
    for i in range(film_count):
        title = titles[i] if i < len(titles) else (
            f"{titles[i % len(titles)]} - {modifiers[(i // len(titles)) % len(modifiers)]}"
            f" - Chương {i + 1:06d}"
        )
        films.append((i + 1, title, 90 + (i * 7) % 71))
    rooms = [(i, f"Phòng {i:03d}", 10, 10) for i in range(1, 21)]
    seats = [(room * 100 + row * 10 + column + 1, room + 1, row + 1, column + 1)
             for room in range(20) for row in range(10) for column in range(10)]
    start = datetime(2026, 11, 29, 9)
    screenings = []
    for i in range(2000):
        slot = i // 20
        time = start + timedelta(days=slot // 3, hours=(slot % 3) * 4)
        screenings.append((i + 1, i % film_count + 1, i % 20 + 1,
                           time.strftime("%Y-%m-%d %H:%M"), 60000 + (i % 5) * 10000))
    tickets = []
    for i in range(5000):
        screening = screenings[i // 5]
        seat_id = (screening[2] - 1) * 100 + i % 5 + 1
        sold = datetime(2026, 11, 28, 8) + timedelta(minutes=i % 600)
        tickets.append((i + 1, screening[0], seat_id, sold.strftime("%Y-%m-%d %H:%M"), screening[4]))

    tables = {
        "films.txt": ("film_id|title|duration_minutes", films),
        "auditoriums.txt": ("auditorium_id|name|row_count|seats_per_row", rooms),
        "seats.txt": ("seat_id|auditorium_id|row_number|seat_number", seats),
        "screenings.txt": ("screening_id|film_id|auditorium_id|start_time|ticket_price", screenings),
        "tickets.txt": ("ticket_id|screening_id|seat_id|sold_at|unit_price", tickets),
    }
    destination = Path(destination)
    destination.mkdir(parents=True, exist_ok=False)
    for filename, (header, rows) in tables.items():
        with (destination / filename).open("x", encoding="utf-8", newline="") as output:
            output.write(header + "\n")
            for row in rows:
                output.write("|".join(map(str, row)) + "\n")

    # The current application has no User entity. These fixtures are not loaded by it.
    fixtures = destination / "fixtures"
    fixtures.mkdir()
    with (fixtures / "users.csv").open("x", encoding="utf-8", newline="") as output:
        writer = csv.writer(output)
        writer.writerow(["user_id", "display_name", "email"])
        for i in range(1, 1001):
            writer.writerow([i, f"Khách mô phỏng {i:04d}", f"guest{i:04d}@example.invalid"])
    with (fixtures / "ticket_users.csv").open("x", encoding="utf-8", newline="") as output:
        writer = csv.writer(output)
        writer.writerow(["ticket_id", "user_id"])
        writer.writerows((i, (i - 1) % 1000 + 1) for i in range(1, 5001))
    manifest = {
        "generator": "tools/generate_sample_data.py", "deterministic": True,
        "counts": {name: len(rows) for name, (_, rows) in tables.items()},
        "fixture_users": 1000, "fixture_ticket_users": 5000,
        "note": "users are synthetic fixtures only, not an application entity or login accounts",
        "sale_date": "2026-11-28", "first_screening": "2026-11-29 09:00",
    }
    with (destination / "manifest.json").open("x", encoding="utf-8") as output:
        json.dump(manifest, output, ensure_ascii=False, indent=2)
        output.write("\n")
    return manifest


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("destination", type=Path, help="New directory, e.g. data/sample")
    parser.add_argument("--films", type=int, default=1000)
    args = parser.parse_args()
    print(json.dumps(generate(args.destination, args.films), ensure_ascii=False, indent=2))
