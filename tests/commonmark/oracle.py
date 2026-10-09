"""Independent, deliberately bounded projection of official HTML expectations.

This module never reads production output or reparses Markdown. Unsupported or
ambiguous HTML is reported instead of repaired like a browser would repair it.
"""
from dataclasses import dataclass, field
from html import unescape
from html.parser import HTMLParser
import re
from urllib.parse import quote


class Uncheckable(ValueError):
    """An official HTML expectation cannot be projected by this oracle."""


def utf16(text):
    return len(text.encode("utf-16-le")) // 2


@dataclass
class Node:
    tag: str
    attrs: dict = field(default_factory=dict)
    children: list = field(default_factory=list)
    opening: str = ""
    closing: str = ""


@dataclass
class Entity:
    text: str


@dataclass
class Literal:
    text: str


class Tree(HTMLParser):
    def __init__(self, source):
        super().__init__(convert_charrefs=False)
        # Short-comment recovery differs across Python releases. Do not let a
        # browser-style recovery rule invent a native literal-HTML expectation.
        if "<!-->" in source or "<!--->" in source:
            raise Uncheckable("short-html-comment-tokenization-unavailable")
        self.source = source
        self.offsets = [0]
        for match in re.finditer("\n", source):
            self.offsets.append(match.end())
        self.spans = []
        self.root = Node("root")
        self.stack = [self.root]
        self.feed(source)
        self.close()
        cursor = 0
        for start, end in sorted(self.spans):
            if start != cursor:
                raise Uncheckable("incomplete-or-ambiguous-html-tokenization")
            cursor = end
        if cursor != len(source):
            raise Uncheckable("incomplete-or-ambiguous-html-tokenization")
        if len(self.stack) != 1:
            raise Uncheckable("unclosed-or-ambiguous-html")

    def record(self, raw):
        line, column = self.getpos()
        start = self.offsets[line - 1] + column
        if self.source[start:start + len(raw)] != raw:
            raise Uncheckable("incomplete-or-ambiguous-html-tokenization")
        self.spans.append((start, start + len(raw)))
        return raw

    def token(self, terminator=">"):
        line, column = self.getpos()
        start = self.offsets[line - 1] + column
        end = self.source.find(terminator, start)
        if end < 0:
            raise Uncheckable("incomplete-html-token")
        return self.record(self.source[start:end + len(terminator)])

    def handle_starttag(self, tag, attrs):
        if len(dict(attrs)) != len(attrs):
            raise Uncheckable("duplicate-html-attributes")
        node = Node(tag, dict(attrs), opening=self.record(self.get_starttag_text()))
        self.stack[-1].children.append(node)
        if tag not in {"img", "br", "hr"}:
            self.stack.append(node)

    def handle_startendtag(self, tag, attrs):
        self.handle_starttag(tag, attrs)
        if tag not in {"img", "br", "hr"}:
            self.stack.pop()

    def handle_endtag(self, tag):
        if len(self.stack) > 1 and self.stack[-1].tag == tag:
            self.stack.pop().closing = self.token()
        else:
            raise Uncheckable("unmatched-or-ambiguous-html-tag")

    def handle_data(self, data):
        self.record(data)
        self.stack[-1].children.append(data)

    def handle_entityref(self, name):
        self.record("&" + name + ";")
        self.stack[-1].children.append(Entity(unescape("&" + name + ";")))

    def handle_charref(self, name):
        self.record("&#" + name + ";")
        self.stack[-1].children.append(Entity(unescape("&#" + name + ";")))

    def handle_comment(self, data):
        self.stack[-1].children.append(Literal(self.token("-->")))

    def handle_decl(self, decl):
        self.stack[-1].children.append(Literal(self.token()))

    def handle_pi(self, data):
        self.stack[-1].children.append(Literal(self.token()))

    def unknown_decl(self, data):
        raise Uncheckable("unsupported-html-declaration")


BLOCKS = {"p", "h1", "h2", "h3", "h4", "h5", "h6", "pre", "hr", "ul", "ol", "li", "blockquote"}
INLINE = {"em", "strong", "code", "a", "img", "br"}


def attributes(node, allowed, required=()):
    if set(node.attrs) - set(allowed) or set(required) - set(node.attrs):
        raise Uncheckable("raw-or-unsupported-html-attributes")
    if any(value is None for value in node.attrs.values()):
        raise Uncheckable("valueless-html-attribute")


def encoded_destination(value):
    # Test comparison only: retain the probe's original URL in its raw output.
    # Both literal Unicode and existing % escapes must match the HTML renderer.
    return quote(value, safe="-_.+!*'(),%#@?=;:/&$~")


def normalize_inline(nodes):
    """Only adjacent Text siblings may coalesce; containers and breaks survive."""
    result = []
    for node in nodes:
        value = dict(node)
        if "children" in value:
            value["children"] = normalize_inline(value["children"])
        if value["kind"] == "Text":
            if result and result[-1]["kind"] == "Text":
                result[-1]["literal"] += value["literal"]
                continue
        result.append(value)
    return result


def semantic_html(items):
    """Retain the HTML evidence's containers, never infer Markdown from output."""
    result = []
    for item in items:
        if isinstance(item, str):
            parts = item.split("\n")
            for index, part in enumerate(parts):
                if index:
                    result.append({"kind": "SoftBreak"})
                if part:
                    result.append({"kind": "Text", "literal": part})
        elif isinstance(item, Entity):
            result.append({"kind": "Text", "literal": item.text})
        elif isinstance(item, Literal):
            result.append({"kind": "Html", "literal": item.text})
        elif item.tag in {"em", "strong", "a"}:
            value = {"kind": {"em": "Emphasis", "strong": "Strong", "a": "Link"}[item.tag],
                     "children": semantic_html(item.children)}
            if item.tag == "a":
                value.update(destination=item.attrs["href"], title=item.attrs.get("title", ""))
            result.append(value)
        elif item.tag == "code":
            result.append({"kind": "Code", "literal": "".join(c.text if isinstance(c, Entity) else c for c in item.children)})
        elif item.tag == "br":
            result.append({"kind": "HardBreak"})
        elif item.tag == "img":
            # HTML exposes only rendered alt text. Interior source nodes are checked
            # by source-authored fixtures and parser tests, not invented here.
            result.append({"kind": "Image", "destination": item.attrs["src"],
                           "title": item.attrs.get("title", ""), "description": item.attrs["alt"]})
        else:
            result.append({"kind": "Html", "literal": item.opening})
            result.extend(semantic_html(item.children))
            if item.closing:
                result.append({"kind": "Html", "literal": item.closing})
    return normalize_inline(result)


def image_description(nodes):
    result = ""
    for node in nodes:
        kind = node["kind"]
        if kind in {"Text", "Code", "Html"}:
            result += node["literal"]
        elif kind in {"SoftBreak", "HardBreak"}:
            result += "\n"
        else:
            result += image_description(node["children"])
    return result


def canonical_inline(nodes):
    result = []
    for node in nodes:
        value = dict(node)
        if value["kind"] == "Image":
            value["description"] = image_description(value.pop("children"))
        elif "children" in value:
            value["children"] = canonical_inline(value["children"])
        if "destination" in value:
            value["destination"] = encoded_destination(value["destination"])
        result.append(value)
    return normalize_inline(result)


class Projection:
    def __init__(self):
        self.limits = set()
        self.losses = set()

    def inline(self, children):
        result = {"text": "", "ranges": [], "links": [], "images": []}

        def append(text, flags, literal=False):
            if not literal and "\n\n" in text:
                raise Uncheckable("literal-newline-or-softbreak-ambiguous")
            if not literal and "\n" in text:
                self.limits.add("softbreak-or-decoded-lf-source-unavailable")
                text = text.replace("\n", " ")
            start = utf16(result["text"])
            result["text"] += text
            length = utf16(text)
            if flags and length:
                ranges = result["ranges"]
                if ranges and ranges[-1]["flags"] == flags and ranges[-1]["start"] + ranges[-1]["length"] == start:
                    ranges[-1]["length"] += length
                else:
                    ranges.append({"start": start, "length": length, "flags": flags})

        def visit(items, flags=0, enclosing=None):
            for item in items:
                if isinstance(item, str):
                    append(item, flags)
                    continue
                if isinstance(item, Entity):
                    append(item.text, flags, literal=True)
                    continue
                if isinstance(item, Literal):
                    append(item.text, flags, literal=True)
                    continue
                if item.tag in BLOCKS:
                    raise Uncheckable("block-tag-in-inline-context")
                if item.tag not in INLINE:
                    # Unknown balanced inline tags have an exact literal spelling.
                    append(item.opening, flags, literal=True)
                    visit(item.children, flags, enclosing)
                    append(item.closing, flags, literal=True)
                    continue
                self.limits.add("inline-html-or-generated-tag-source-unavailable")
                if item.tag in {"em", "strong"}:
                    attributes(item, ())
                    bit = 1 if item.tag == "em" else 2
                    visit(item.children, flags | bit, enclosing)
                elif item.tag == "code":
                    attributes(item, ())
                    if any(not isinstance(c, (str, Entity)) for c in item.children):
                        raise Uncheckable("nontext-code-span")
                    append("".join(c.text if isinstance(c, Entity) else c for c in item.children), flags | 4, literal=True)
                elif item.tag == "br":
                    attributes(item, ())
                    append("\n", flags, literal=True)
                    # The LF after <br /> is HTML serialization, not a soft break.
                elif item.tag == "a":
                    attributes(item, ("href", "title"), ("href",))
                    if enclosing is not None:
                        raise Uncheckable("nested-html-links")
                    start = utf16(result["text"])
                    url = item.attrs["href"]
                    self.limits.add("url-original-escape-spelling-unavailable")
                    visit(item.children, flags, url)
                    length = utf16(result["text"]) - start
                    if length:
                        result["links"].append({"start": start, "length": length, "destination": url})
                elif item.tag == "img":
                    attributes(item, ("src", "alt", "title"), ("src", "alt"))
                    if item.children:
                        raise Uncheckable("image-has-children")
                    start = utf16(result["text"])
                    append(item.attrs["alt"], flags)
                    result["images"].append({"start": start, "length": utf16(result["text"]) - start,
                        "destination": item.attrs["src"], "title": item.attrs.get("title", ""),
                        "enclosingLink": enclosing or "", "linked": enclosing is not None})
                    self.limits.update({"image-description-formatting-unavailable", "url-original-escape-spelling-unavailable"})

        # Remove only the serializer's LF immediately following a br node.
        def breaks(items):
            output = []
            for item in items:
                if isinstance(item, str) and output and isinstance(output[-1], Node) and output[-1].tag == "br" and item.startswith("\n"):
                    item = item[1:]
                if isinstance(item, Node):
                    item.children = breaks(item.children)
                output.append(item)
            return output

        children = breaks(children)
        visit(children)
        result["inlines"] = semantic_html(children)
        return result

    def blocks(self, children, item_context=False):
        result = []
        pending = []

        def flush(before_block=False):
            nonlocal pending
            if not pending:
                return
            if item_context and result and isinstance(pending[0], str) and pending[0].startswith("\n"):
                pending[0] = pending[0][1:]
            # cmark adds a separating LF before/after nested block children of li.
            if before_block and isinstance(pending[-1], str) and pending[-1].endswith("\n"):
                pending[-1] = pending[-1][:-1]
            if all(isinstance(c, str) and not c.strip() for c in pending):
                pending = []
                return
            if not item_context:
                raise Uncheckable("raw-html-or-unwrapped-block-text")
            result.append({"kind": "Paragraph", **self.inline(pending)})
            pending = []

        for child in children:
            if not isinstance(child, Node) or child.tag not in BLOCKS:
                pending.append(child)
                continue
            flush(before_block=True)
            tag = child.tag
            if tag == "p" or re.fullmatch("h[1-6]", tag):
                attributes(child, ())
                value = {"kind": "Paragraph" if tag == "p" else "Heading", **self.inline(child.children)}
                if tag != "p":
                    value["level"] = int(tag[1])
            elif tag == "pre":
                attributes(child, ())
                if len(child.children) != 1 or not isinstance(child.children[0], Node) or child.children[0].tag != "code":
                    raise Uncheckable("raw-or-unsupported-pre")
                code = child.children[0]
                attributes(code, ("class",))
                if any(not isinstance(c, (str, Entity)) for c in code.children):
                    raise Uncheckable("nontext-code-block")
                language = code.attrs.get("class", "")
                if language and not language.startswith("language-"):
                    raise Uncheckable("unsupported-code-class")
                value = {"kind": "CodeBlock", "text": "".join(c.text if isinstance(c, Entity) else c for c in code.children), "language": language[9:] if language else ""}
                self.limits.add("fence-info-tail-and-code-origin-unavailable")
            elif tag == "hr":
                attributes(child, ())
                value = {"kind": "ThematicBreak"}
            elif tag == "blockquote":
                attributes(child, ())
                value = {"kind": "Quote", "children": self.blocks(child.children)}
            elif tag in {"ul", "ol"}:
                attributes(child, ("start",) if tag == "ol" else ())
                items = [c for c in child.children if not (isinstance(c, str) and not c.strip())]
                if any(not isinstance(c, Node) or c.tag != "li" for c in items):
                    raise Uncheckable("nonitem-list-child")
                tight = not any(isinstance(c, Node) and c.tag == "p" for li in items for c in li.children)
                value = {"kind": "List", "ordered": tag == "ol", "tight": tight,
                         "children": [self.list_item(li) for li in items]}
                if tag == "ol":
                    try:
                        value["start"] = int(child.attrs.get("start", "1"))
                    except ValueError as error:
                        raise Uncheckable("invalid-list-start") from error
                    self.limits.add("ordered-list-delimiter-unavailable")
            else:
                raise Uncheckable("list-item-outside-list")
            result.append(value)
        flush(before_block=item_context and any(isinstance(c, Node) and c.tag in BLOCKS for c in children))
        return result

    def list_item(self, node):
        attributes(node, ())
        return {"kind": "ListItem", "children": self.blocks(node.children, item_context=True)}


def mask_image_ranges(block):
    """Exclude only description interiors, not adjacent or surrounding text."""
    ranges = []
    for span in block["ranges"]:
        pieces = [(span["start"], span["start"] + span["length"])]
        for image in block["images"]:
            low, high = image["start"], image["start"] + image["length"]
            pieces = [(a, b) for start, end in pieces
                      for a, b in ((start, min(end, low)), (max(start, high), end)) if b > a]
        for start, end in pieces:
            if ranges and ranges[-1]["flags"] == span["flags"] and ranges[-1]["start"] + ranges[-1]["length"] == start:
                ranges[-1]["length"] += end - start
            else:
                ranges.append({"start": start, "length": end - start, "flags": span["flags"]})
    return {**block, "ranges": ranges}


def canonical_actual(blocks):
    result = []
    for block in blocks:
        value = dict(block)
        if "children" in value:
            value["children"] = canonical_actual(value["children"])
        if value["kind"] == "CodeBlock":
            info = value.pop("infoString")
            value["language"] = info.split()[0] if info.split() else ""
        if value["kind"] == "List":
            value.pop("delimiter")
            if not value["ordered"]:
                value.pop("start")
        if "images" in value:
            value["inlines"] = canonical_inline(value["inlines"])
            value = mask_image_ranges(value)
            value["links"] = [{**link, "destination": encoded_destination(link["destination"])} for link in value["links"]]
            value["images"] = [{**image, "destination": encoded_destination(image["destination"]),
                "enclosingLink": encoded_destination(image["enclosingLink"])} for image in value["images"]]
        result.append(value)
    return result


def expected(html):
    projection = Projection()
    blocks = projection.blocks(Tree(html).root.children)
    return canonical_expected(blocks), sorted(projection.limits), sorted(projection.losses)


def canonical_expected(blocks):
    result = []
    for block in blocks:
        value = dict(block)
        if "children" in value:
            value["children"] = canonical_expected(value["children"])
        if "images" in value:
            value = mask_image_ranges(value)
        result.append(value)
    return result
