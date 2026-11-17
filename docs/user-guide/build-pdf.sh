#!/usr/bin/env bash
# Builds a single PDF from every article under docs/user-guide/, in the same
# order and grouping as index.md. Requires pandoc + a LaTeX engine (xelatex):
#   sudo apt install pandoc texlive-xetex
# Usage: build-pdf.sh [output.pdf]   (default: docs/user-guide/antenna-osp-User-Guide.pdf)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Resolve a relative $1 against the caller's cwd, before it changes below.
OUTPUT="${1:-$SCRIPT_DIR/Antenna-OSP-User-Guide.pdf}"
case "$OUTPUT" in
  /*) ;;
  *) OUTPUT="$(pwd)/$OUTPUT" ;;
esac
mkdir -p "$(dirname "$OUTPUT")"

cd "$SCRIPT_DIR"

for cmd in pandoc xelatex python3; do
  command -v "$cmd" >/dev/null 2>&1 || {
    echo "Missing dependency: $cmd (try: sudo apt install pandoc texlive-xetex)" >&2
    exit 1
  }
done

WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

# --- Step 1: concatenate index.md's intro + every linked article, in its
#     order, into one Markdown file. Image paths lose their "../" since pandoc
#     resolves relative paths against the working directory once merged.
python3 - "$SCRIPT_DIR" "$WORK_DIR/combined.md" <<'PYEOF'
import re, sys

root, out_path = sys.argv[1], sys.argv[2]

with open(f"{root}/index.md") as f:
    index_content = f.read()

# Preface: the welcome paragraph between the H1 and the first "## " section.
preface = re.search(r"^# .*\n(.*?)\n## ", index_content, re.S | re.M).group(1).strip()

# Article order: every "(folder/name.md)" link in index.md, in reading order.
# _open-questions.md is not in a folder, so it stays out of the PDF.
order = re.findall(r"\]\(([a-z]+/[a-z0-9-]+\.md)\)", index_content)
if not order:
    sys.exit("No article links found in index.md - check its format hasn't changed.")

parts = ["# Introduction\n\n" + preface + "\n"]
for path in order:
    with open(f"{root}/{path}") as f:
        content = f.read()
    content = content.replace("(../images/", "(images/")
    parts.append(content.strip() + "\n")

with open(out_path, "w") as f:
    f.write("\n\n".join(parts))

with open(f"{out_path}.order", "w") as f:
    f.write("\n".join(order))
PYEOF

# --- Step 2: render once to HTML just to read pandoc's auto-generated header
#     ids, then rewrite every inter-article ".md" link into a "#that-id" anchor
#     so they work as internal PDF links instead of dead file references.
pandoc "$WORK_DIR/combined.md" -f markdown -t html --resource-path=. \
  -o "$WORK_DIR/combined_probe.html"

python3 - "$WORK_DIR/combined.md" "$WORK_DIR/combined_probe.html" "$WORK_DIR/combined.md.order" "$WORK_DIR/combined_linked.md" <<'PYEOF'
import re, sys

md_path, html_path, order_path, out_path = sys.argv[1:5]

with open(order_path) as f:
    order = [line.strip() for line in f if line.strip()]

with open(html_path) as f:
    html = f.read()

# One <h1 id="..."> per chapter, in document order; index 0 is the
# "Introduction" chapter this script adds, so real articles start at 1.
h1_ids = re.findall(r'<h1 id="([^"]+)">', html)
if len(h1_ids) != len(order) + 1:
    sys.exit(
        f"Expected {len(order) + 1} top-level headings (Introduction + "
        f"{len(order)} articles), found {len(h1_ids)} - a probe/article "
        "mismatch means a rewritten link would point at the wrong chapter."
    )

slug_map = {path.rsplit("/", 1)[-1]: slug for path, slug in zip(order, h1_ids[1:])}

with open(md_path) as f:
    content = f.read()

# "](any/path/FILENAME.md)" -> "](#slug)"; a "#anchor" suffix is kept as-is.
pattern = re.compile(r"\]\((?:\.\./[a-z]+/|[a-z-]+/)?([a-z0-9-]+\.md)(#[a-z0-9-]+)?\)")

def repl(m):
    filename, anchor = m.group(1), m.group(2)
    slug = slug_map.get(filename)
    if slug is None:
        return m.group(0)
    return f"]({anchor if anchor else '#' + slug})"

with open(out_path, "w") as f:
    f.write(pattern.sub(repl, content))
PYEOF

# --- Step 3: constrain embedded screenshots to the text width.
cat > "$WORK_DIR/header.tex" <<'TEX'
\usepackage{graphicx}
\makeatletter
\def\maxwidth{\ifdim\Gin@nat@width>\linewidth\linewidth\else\Gin@nat@width\fi}
\def\maxheight{\ifdim\Gin@nat@height>0.8\textheight 0.8\textheight\else\Gin@nat@height\fi}
\makeatother
\setkeys{Gin}{width=\maxwidth,height=\maxheight,keepaspectratio}
TEX

# --- Step 4: render the final PDF. Each article's "# Title" becomes its own
#     LaTeX chapter (page break + numbering + top-level TOC entry).
pandoc "$WORK_DIR/combined_linked.md" \
  -o "$OUTPUT" \
  --pdf-engine=xelatex \
  --resource-path="$SCRIPT_DIR" \
  --toc --toc-depth=1 \
  --top-level-division=chapter \
  -V documentclass=report \
  -V geometry:margin=1in \
  -V geometry:a4paper \
  -V colorlinks=true \
  -V linkcolor=blue \
  -V toccolor=black \
  --include-in-header="$WORK_DIR/header.tex" \
  -M title="Antenna OSP User Guide" \
  -M subtitle="Desktop RF switch, OSP clone" \
  -M date="$(date +%d.%m.%Y)" \
  -M author="Michael Wolf, Mictronics" \
  -M keywords="Antenna-OSP, RF switch, User Guide"

echo "Built $OUTPUT"
