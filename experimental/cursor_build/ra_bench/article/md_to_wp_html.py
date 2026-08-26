"""Convert neoclassical_cpp_2.md to native Gutenberg block markup."""
from __future__ import annotations

import json
import re
from pathlib import Path

import markdown
from bs4 import BeautifulSoup, NavigableString, Tag

SRC = Path(__file__).with_name("neoclassical_cpp_2.md")
DST = Path(__file__).with_name("neoclassical_cpp_2.gutenberg.html")

CODE_BG = "#f3f4f6"  # used in suggested Additional CSS for .wp-block-code


def attrs(obj: dict | None) -> str:
    if not obj:
        return ""
    return " " + json.dumps(obj, separators=(",", ":"), ensure_ascii=False)


def block(name: str, inner: str, attributes: dict | None = None) -> str:
    return f"<!-- wp:{name}{attrs(attributes)} -->\n{inner}\n<!-- /wp:{name} -->\n"


def escape_img_alts(text: str) -> str:
    def repl(m: re.Match[str]) -> str:
        pre, alt, post = m.group(1), m.group(2), m.group(3)
        alt = (
            alt.replace("&lt;", "<")
            .replace("&gt;", ">")
            .replace("&amp;", "&")
            .replace("&", "&amp;")
            .replace("<", "&lt;")
            .replace(">", "&gt;")
        )
        return f"{pre}{alt}{post}"

    return re.sub(r'(<img\b[^>]*\balt=")([^"]*)(")', repl, text)


def inline_html(node: Tag | NavigableString) -> str:
    if isinstance(node, NavigableString):
        return str(node)
    return "".join(str(c) for c in node.children)


def minify_tag_html(html: str) -> str:
    """Remove whitespace between tags so table/list save HTML validates."""
    return re.sub(r">\s+<", "><", html.strip())


def li_content(li: Tag) -> str:
    parts = []
    for child in li.children:
        if isinstance(child, Tag) and child.name == "p":
            parts.append(inline_html(child).strip())
        elif isinstance(child, NavigableString):
            s = str(child).strip()
            if s:
                parts.append(s)
        else:
            parts.append(inline_html(child).strip())
    return " ".join(p for p in parts if p)


def convert_list(ul: Tag) -> str:
    ordered = ul.name == "ol"
    items = []
    for li in ul.find_all("li", recursive=False):
        content = li_content(li)
        # Compact list-item serialization (matches core paste output).
        items.append(f"<!-- wp:list-item -->\n<li>{content}</li>\n<!-- /wp:list-item -->")
    tag = "ol" if ordered else "ul"
    # No newlines between </li> comment and next item / closing tag.
    inner = "".join(items)
    attributes = {"ordered": True} if ordered else None
    return block(
        "list",
        f'<{tag} class="wp-block-list">{inner}</{tag}>',
        attributes,
    )


def convert_table(table: Tag) -> str:
    # core/table validation is brittle with nested <code> etc.; Custom HTML always validates.
    compact = minify_tag_html(str(table))
    return block("html", compact)


def wp_escape_code(text: str) -> str:
    """Match core/code save(): escapeHTML + shortcode/URL escaping.

    WordPress escapeHTML only escapes & and < — not >.
    See @wordpress/escape-html escapeHTML and block-library/src/code/utils.js.
    """
    # All ampersands first (editable HTML style), then <.
    text = text.replace("&", "&amp;").replace("<", "&lt;")
    # [ => &#91; so shortcodes are not parsed.
    text = text.replace("[", "&#91;")
    # Isolated https?:// on its own line.
    text = re.sub(
        r"^(\s*https?:)//([^\s<>\"]+\s*)$",
        r"\1&#47;&#47;\2",
        text,
        flags=re.M,
    )
    return text


def convert_pre(pre: Tag) -> str:
    code = pre.find("code")
    text = code.get_text() if code else pre.get_text()
    if text.endswith("\n"):
        text = text[:-1]
    # Normalize to LF — CRLF inside code breaks validation after paste.
    text = text.replace("\r\n", "\n").replace("\r", "\n")
    text = wp_escape_code(text)

    language = "cpp"
    if code is not None:
        for cls in code.get("class", []) or []:
            if cls.startswith("language-"):
                language = cls[len("language-") :] or "cpp"
                break

    return block(
        "code",
        f'<pre class="wp-block-code"><code>{text}</code></pre>',
        {"language": language},
    )


def convert_img(img: Tag) -> str:
    src = img.get("src", "")
    alt_raw = img.get("alt", "") or ""
    alt_text = BeautifulSoup(alt_raw, "html.parser").get_text()
    alt_html = (
        alt_text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
    )
    # `url` / `alt` in attributes are required for image block validation.
    return block(
        "image",
        f'<figure class="wp-block-image aligncenter size-full">'
        f'<img src="{src}" alt="{alt_html}"></figure>',
        {
            "url": src,
            "alt": alt_text,
            "align": "center",
            "sizeSlug": "full",
            "linkDestination": "none",
        },
    )


def convert_heading(h: Tag) -> str:
    level = int(h.name[1])
    inner = inline_html(h)
    # Default heading level in core is 2 — omit attribute for h2.
    attributes = None if level == 2 else {"level": level}
    return block(
        "heading",
        f'<h{level} class="wp-block-heading">{inner}</h{level}>',
        attributes,
    )


def convert_paragraph(p: Tag) -> str:
    imgs = [c for c in p.children if isinstance(c, Tag) and c.name == "img"]
    # Sole child is an image → image block.
    significant = [
        c
        for c in p.children
        if not (isinstance(c, NavigableString) and not str(c).strip())
    ]
    if len(significant) == 1 and isinstance(significant[0], Tag) and significant[0].name == "img":
        return convert_img(significant[0])
    if len(imgs) == 1 and not inline_html(p).replace(str(imgs[0]), "").strip():
        return convert_img(imgs[0])
    return block("paragraph", f"<p>{inline_html(p)}</p>")


def convert_hr(_: Tag) -> str:
    return block(
        "separator",
        '<hr class="wp-block-separator has-alpha-channel-opacity"/>',
    )


def convert(soup: BeautifulSoup) -> str:
    out: list[str] = []
    for el in soup.children:
        if isinstance(el, NavigableString):
            if str(el).strip():
                out.append(block("paragraph", f"<p>{str(el).strip()}</p>"))
            continue
        if not isinstance(el, Tag):
            continue
        name = el.name
        if name in {"h1", "h2", "h3", "h4", "h5", "h6"}:
            out.append(convert_heading(el))
        elif name == "p":
            out.append(convert_paragraph(el))
        elif name in {"ul", "ol"}:
            out.append(convert_list(el))
        elif name == "pre":
            out.append(convert_pre(el))
        elif name == "table":
            out.append(convert_table(el))
        elif name == "hr":
            out.append(convert_hr(el))
        elif name == "img":
            out.append(convert_img(el))
        elif name == "blockquote":
            out.append(
                block(
                    "quote",
                    f'<blockquote class="wp-block-quote">{inline_html(el)}</blockquote>',
                )
            )
        else:
            out.append(block("html", str(el)))
    return "\n".join(out)


def main() -> None:
    md = escape_img_alts(SRC.read_text(encoding="utf-8"))
    html = markdown.markdown(
        md,
        extensions=["tables", "fenced_code", "sane_lists"],
        output_format="html5",
    )
    soup = BeautifulSoup(html, "html.parser")
    gutenberg = convert(soup)
    DST.write_text(gutenberg, encoding="utf-8")
    print(f"wrote {DST} ({DST.stat().st_size} bytes)")
    for kind in ("heading", "paragraph", "image", "table", "code", "list", "separator", "html"):
        # Count only exact block openers (not list-item).
        n = len(re.findall(rf"<!-- wp:{kind}(?: |-->)", gutenberg))
        print(f"  wp:{kind}: {n}")


if __name__ == "__main__":
    main()
