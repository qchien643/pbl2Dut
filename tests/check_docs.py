"""Run with python3 tests/check_docs.py; standard library only, no build needed."""

from html.parser import HTMLParser
from pathlib import Path
import re
from urllib.parse import unquote, urlsplit


ROOT = Path(__file__).resolve().parents[1]


def check_link(source, target):
    url = urlsplit(target)
    if not url.scheme and not url.netloc and url.path:
        path = source.parent / unquote(url.path)
        assert path.exists(), f"{source.relative_to(ROOT)}: missing {target}"


class Diagram(HTMLParser):
    def __init__(self, path):
        super().__init__(convert_charrefs=True)
        self.path = path
        self.ids = set()
        self.svg = []
        self.current_svg = None

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if "id" in attrs:
            assert attrs["id"] not in self.ids, f"{self.path}: duplicate ID"
            self.ids.add(attrs["id"])
        for key in ("href", "src"):
            if key in attrs:
                check_link(self.path, attrs[key])
        if tag == "svg":
            self.current_svg = {"attrs": attrs, "labels": {}}
            self.svg.append(self.current_svg)
        if self.current_svg is not None and tag in ("title", "desc"):
            self.current_svg["labels"][tag] = attrs.get("id")

    def handle_endtag(self, tag):
        if tag == "svg":
            self.current_svg = None


def main():
    markdown = [ROOT / "README.md"] + sorted((ROOT / "docs").rglob("*.md"))
    count = 0
    for path in markdown:
        if {"slides", "ref"} & set(path.relative_to(ROOT).parts):
            continue
        content = re.sub(r"```.*?```", "", path.read_text(encoding="utf-8"), flags=re.S)
        for target in re.findall(r"!?\[[^\]]*\]\(([^\s)]+)\)", content):
            check_link(path, target)
        count += 1
    diagrams = sorted((ROOT / "docs/diagrams").rglob("*.html"))
    assert diagrams, "No current diagrams found"
    for path in diagrams:
        description = path.parent / "README.md"
        assert path.parent.name == path.stem, f"{path}: expected a same-name folder"
        assert description.is_file(), f"{path}: missing Markdown description"
        assert f"]({path.name})" in description.read_text(encoding="utf-8"), description
        parser = Diagram(path)
        parser.feed(path.read_text(encoding="utf-8"))
        assert parser.svg, f"{path}: expected at least one SVG"
        for svg in parser.svg:
            attrs, labels = svg["attrs"], svg["labels"]
            assert attrs.get("role") == "img", path
            title, desc = labels.get("title", ""), labels.get("desc", "")
            assert title.startswith(path.stem + "-") and title.endswith("-title"), path
            assert desc == title.removesuffix("-title") + "-desc", path
            assert attrs.get("aria-labelledby") == f"{title} {desc}", path
    print(f"OK: {count} Markdown files, {len(diagrams)} diagrams; local links and SVG labels")


if __name__ == "__main__":
    main()
