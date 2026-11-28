"""Run with python3 tests/check_sample_data.py; checks fixtures and generator safety."""

import csv
import json
from pathlib import Path
import runpy
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SAMPLE = ROOT / "data/sample"


def rows(path, delimiter="|"):
    with path.open(encoding="utf-8", newline="") as stream:
        return list(csv.DictReader(stream, delimiter=delimiter))


def main():
    manifest = json.loads((SAMPLE / "manifest.json").read_text(encoding="utf-8"))
    tables = {}
    for filename, count in manifest["counts"].items():
        records = rows(SAMPLE / filename)
        assert len(records) == count
        primary = next(iter(records[0]))
        assert len({record[primary] for record in records}) == count
        tables[filename] = records
    assert manifest["counts"]["films.txt"] == 1000
    assert len({film["title"] for film in tables["films.txt"]}) == 1000
    users = rows(SAMPLE / "fixtures/users.csv", ",")
    purchases = rows(SAMPLE / "fixtures/ticket_users.csv", ",")
    user_ids = {row["user_id"] for row in users}
    ticket_ids = {row["ticket_id"] for row in tables["tickets.txt"]}
    assert len(users) == len(user_ids) == 1000
    assert len(purchases) == len(ticket_ids) == 5000
    assert {row["ticket_id"] for row in purchases} == ticket_ids
    assert {row["user_id"] for row in purchases} == user_ids
    assert all(row["email"].endswith("@example.invalid") for row in users)
    for purchase in purchases:
        assert purchase["ticket_id"] in ticket_ids and purchase["user_id"] in user_ids

    generate = runpy.run_path(str(ROOT / "tools/generate_sample_data.py"))["generate"]
    with tempfile.TemporaryDirectory(prefix="pbl2-generator-test-") as temporary:
        destination = Path(temporary) / "sample"
        generate(destination)
        for source in SAMPLE.rglob("*"):
            if source.is_file() and source.name != "README.md":
                assert source.read_bytes() == (destination / source.relative_to(SAMPLE)).read_bytes()
        before = (destination / "films.txt").read_bytes()
        try:
            generate(destination)
        except FileExistsError:
            pass
        else:
            raise AssertionError("Generator overwrote existing data")
        assert (destination / "films.txt").read_bytes() == before
        invalid = Path(temporary) / "invalid"
        try:
            generate(invalid, 0)
        except ValueError:
            pass
        else:
            raise AssertionError("Generator accepted invalid size")
        assert not invalid.exists()
    print("OK: sample counts, user fixtures, relationships, reproducibility and no-overwrite")


if __name__ == "__main__":
    main()
