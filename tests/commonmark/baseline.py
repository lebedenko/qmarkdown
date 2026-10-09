"""Offline CommonMark evidence, strict validation, and reviewed baseline policy."""
import difflib
import hashlib
import json
from pathlib import Path
import platform
import subprocess
from collections import Counter

from oracle import Uncheckable, canonical_actual, expected, utf16

DATA = Path(__file__).resolve().parent
PARSE_OPTIONS = "CMARK_OPT_DEFAULT"
HTML_OPTIONS = "CMARK_OPT_UNSAFE (test serializer only)"


def require(condition, message):
    if not condition:
        raise ValueError(message)


def keys(value, fields, name):
    require(isinstance(value, dict) and set(value) == set(fields), f"Invalid {name} fields")


def integer(value):
    return type(value) is int


def json_text(value):
    return json.dumps(value, ensure_ascii=False, sort_keys=True, indent=2) + "\n"


def digest(value):
    return hashlib.sha256(value).hexdigest()


def validate_fixtures(examples, count):
    require(isinstance(examples, list) and len(examples) == count, "Missing fixture examples")
    ids = []
    for example in examples:
        keys(example, ("example", "markdown", "html", "section", "start_line", "end_line"), "fixture")
        require(integer(example["example"]) and example["example"] > 0, "Invalid fixture ID")
        require(all(isinstance(example[k], str) for k in ("markdown", "html", "section")), "Invalid fixture text")
        require(bool(example["section"]), "Missing fixture section")
        require(integer(example["start_line"]) and integer(example["end_line"])
                and 0 < example["start_line"] <= example["end_line"], "Invalid fixture source lines")
        ids.append(example["example"])
    require(ids == list(range(1, count + 1)), "Missing, duplicate or unordered fixture IDs")


def load_fixtures(directory=DATA):
    manifest = json.loads((directory / "fixture.json").read_text())
    keys(manifest, ("version", "url", "sha256", "count", "license", "author", "acquired"), "fixture manifest")
    require(manifest["version"] == "0.31.2", "Unexpected CommonMark version")
    require(integer(manifest["count"]) and manifest["count"] > 0, "Invalid fixture count")
    require(manifest["license"] == "CC-BY-SA-4.0", "Unexpected fixture license")
    require(manifest["url"] == "https://spec.commonmark.org/0.31.2/spec.json", "Unexpected fixture origin")
    raw = (directory / "spec.json").read_bytes()
    require(digest(raw) == manifest["sha256"], "Fixture checksum mismatch")
    examples = json.loads(raw)
    validate_fixtures(examples, manifest["count"])
    return manifest, examples


def validate_inline(nodes):
    require(isinstance(nodes, list), "Invalid inline array")
    for node in nodes:
        require(isinstance(node, dict), "Invalid inline node")
        kind = node.get("kind")
        require(kind in ("Text", "Code", "Html", "SoftBreak", "HardBreak", "Emphasis", "Strong", "Link", "Image"),
                "Unknown inline kind")
        literal = kind in ("Text", "Code", "Html")
        container = kind in ("Emphasis", "Strong", "Link", "Image")
        target = kind in ("Link", "Image")
        keys(node, ("kind", *(("literal",) if literal else ()), *(("children",) if container else ()),
                    *(("destination", "title") if target else ())), "inline node")
        if literal:
            require(isinstance(node["literal"], str), "Invalid inline literal")
        if container:
            validate_inline(node["children"])
        if target:
            require(all(isinstance(node[k], str) for k in ("destination", "title")), "Invalid inline target")


def validate_model(blocks, context="root"):
    require(isinstance(blocks, list), "Invalid block array")
    fields = {
        "Paragraph": ("kind", "text", "ranges", "links", "images", "inlines"),
        "Heading": ("kind", "level", "text", "ranges", "links", "images", "inlines"),
        "CodeBlock": ("kind", "text", "infoString"), "HtmlBlock": ("kind", "text"),
        "ThematicBreak": ("kind",),
        "List": ("kind", "ordered", "start", "delimiter", "tight", "children"),
        "ListItem": ("kind", "children"), "Quote": ("kind", "children"),
    }
    for block in blocks:
        require(isinstance(block, dict) and isinstance(block.get("kind"), str), "Invalid block kind")
        kind = block["kind"]
        require(kind in fields, "Unknown block kind")
        keys(block, fields[kind], kind)
        require((context == "List") == (kind == "ListItem"), "Invalid list item placement")
        if "text" in block:
            require(isinstance(block["text"], str), "Invalid block text")
        if kind == "Heading":
            require(integer(block["level"]) and 1 <= block["level"] <= 6, "Invalid heading level")
        if kind == "CodeBlock":
            require(isinstance(block["infoString"], str), "Invalid code info")
        if kind == "List":
            require(type(block["ordered"]) is bool and type(block["tight"]) is bool, "Invalid list flags")
            require(integer(block["start"]) and 0 <= block["start"] <= 999999999, "Invalid list start")
            require(block["delimiter"] in (".", ")"), "Invalid list delimiter")
        if "children" in block:
            validate_model(block["children"], kind)
        if kind not in ("Paragraph", "Heading"):
            continue
        validate_inline(block["inlines"])
        size = utf16(block["text"])
        # Boundaries must never bisect an astral character's surrogate pair.
        boundaries = {0}
        offset = 0
        for character in block["text"]:
            offset += 2 if ord(character) > 0xffff else 1
            boundaries.add(offset)
        for category, extra in (("ranges", ("flags",)), ("links", ("destination",)),
                                ("images", ("destination", "title", "enclosingLink", "linked"))):
            require(isinstance(block[category], list), f"Invalid {category}")
            previous = 0
            for span in block[category]:
                keys(span, ("start", "length", *extra), category)
                start, length = span["start"], span["length"]
                require(integer(start) and integer(length) and start >= previous and length >= 0
                        and start + length <= size, f"Invalid {category} range")
                require(start in boundaries and start + length in boundaries, "Split UTF-16 character")
                require(category == "images" or length > 0, "Empty formatting/link range")
                previous = start + length
                if category == "ranges":
                    require(integer(span["flags"]) and 1 <= span["flags"] <= 7, "Invalid inline flags")
                else:
                    require(isinstance(span["destination"], str), "Invalid destination")
                if category == "images":
                    require(all(isinstance(span[k], str) for k in ("title", "enclosingLink"))
                            and type(span["linked"]) is bool, "Invalid image metadata")
                    require(span["linked"] or span["enclosingLink"] == "", "Unexpected unlinked image destination")


HTML_BLOCK_IDS = list(range(148, 168)) + list(range(169, 192))


REMAINING_IDS = [21, 31, 39, 201, 308, 309, 344, 475, 476, 477, 491, 494,
                 524, 536, 613, 614, 615, 616, 623, 626, 628, 629, 630, 631, 642, 643]
AUTHORED_IDS = HTML_BLOCK_IDS + REMAINING_IDS
# Exact iteration 016 ledger union; independent of the now-empty ledger.
SEMANTIC_IDS = [
    1, 2, 3, 5, 6, 7, 8, 14, 15, 16, 17, 18, 19, 20, 22, 23, 24, 25, 28, 32, 33, 34, 35, 36, 37, 46,
    48, 49, 56, 66, 69, 70, 80, 81, 82, 85, 87, 88, 93, 95, 100, 104, 105, 106, 107, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
    131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 168, 192,
    193, 194, 195, 196, 198, 200, 202, 203, 204, 205, 206, 211, 212, 213, 214, 215, 216, 217, 218,
    220, 222, 223, 224, 225, 226, 228, 229, 230, 231, 232, 233, 236, 237, 238, 243, 247, 250, 251,
    252, 253, 254, 257, 259, 263, 264, 265, 267, 268, 270, 271, 272, 273, 274, 278, 283, 285, 286,
    287, 288, 289, 290, 291, 292, 293, 296, 297, 299, 302, 304, 305, 311, 312, 313, 318, 321, 324,
    327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 345, 346,
    349, 350, 355, 356, 357, 364, 367, 369, 370, 373, 376, 377, 378, 381, 382, 384, 389, 390, 393,
    394, 395, 396, 399, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416,
    417, 418, 419, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 437, 438, 440, 441,
    442, 443, 444, 445, 446, 447, 449, 450, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462,
    463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 478, 479, 480, 481, 482, 483, 484,
    485, 486, 487, 489, 490, 492, 495, 496, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 509,
    510, 512, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 525, 526, 527, 528, 529, 530, 531,
    532, 533, 534, 535, 537, 538, 539, 540, 541, 542, 543, 544, 549, 550, 552, 553, 554, 555, 556,
    557, 558, 559, 560, 561, 562, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576,
    577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 591, 593, 594, 595, 596, 597,
    598, 599, 600, 601, 603, 604, 605, 621, 633, 634, 635, 636, 637, 638, 639, 640, 641, 648, 649
]


def validate_authored(value, manifest, ids=HTML_BLOCK_IDS, annotations=False):
    keys(value, ("schema", "fixtureSha256", "review", "examples"), "authored expectations")
    require(type(value["schema"]) is int and value["schema"] == 2
            and value["fixtureSha256"] == manifest["sha256"], "Invalid authored provenance")
    require(isinstance(value["review"], str) and value["review"].strip(), "Missing authored review")
    require(isinstance(value["examples"], list), "Invalid authored examples")
    expected_ids = ids
    ids = []
    for entry in value["examples"]:
        keys(entry, ("example", "review", "model", *(("limits", "losses") if annotations else ())), "authored example")
        if annotations:
            for category in ("limits", "losses"):
                values = entry[category]
                require(isinstance(values, list) and all(isinstance(v, str) and v.strip() for v in values)
                        and values == sorted(set(values)), f"Invalid authored {category}")
        require(integer(entry["example"]), "Invalid authored ID")
        require(isinstance(entry["review"], str) and entry["review"].strip(), "Missing per-ID authored review")
        validate_model(entry["model"])
        ids.append(entry["example"])
    require(ids == expected_ids, "Missing, duplicate or unexpected authored IDs")
    return {e["example"]: ({k: e[k] for k in ("model", "limits", "losses")} if annotations else e["model"])
            for e in value["examples"]}


def load_authored(manifest):
    return validate_authored(json.loads((DATA / "html-block-expectations.json").read_text()), manifest)


def load_remaining(manifest):
    return validate_authored(json.loads((DATA / "remaining-expectations.json").read_text()),
                             manifest, REMAINING_IDS, annotations=True)


def validate_semantic(value, manifest):
    # The new fixture has its own version; existing authored structures stay v2.
    keys(value, ("schema", "fixtureSha256", "review", "examples"), "semantic expectations")
    require(type(value["schema"]) is int and value["schema"] == 1, "Invalid semantic provenance")
    return validate_authored({**value, "schema": 2}, manifest, SEMANTIC_IDS)


def load_semantic(manifest):
    return validate_semantic(json.loads((DATA / "semantic-expectations.json").read_text()), manifest)


def comparison_check(method, want, actual, limits, losses):
    check = {"method": method, "status": "projection-pass" if want == actual else "mismatch",
             "limits": limits, "losses": losses}
    if want != actual:
        check.update(expected=want, actual=actual, diff=difference(json_text(want), json_text(actual)))
    return check


def model_result(fixture, actual, authored, remaining, semantic):
    id_ = fixture["example"]
    checks = []
    if id_ not in AUTHORED_IDS:
        try:
            want, limits, losses = expected(fixture["html"])
            checks.append(comparison_check("html-projection", want, canonical_actual(actual), limits, losses))
        except Uncheckable as error:
            checks.append({"method": "html-projection", "status": "uncheckable", "reason": str(error),
                           "limits": ["html-oracle-unsupported"], "losses": []})
    if id_ in AUTHORED_IDS or id_ in SEMANTIC_IDS:
        if id_ in REMAINING_IDS:
            source = remaining[id_]
            want, limits, losses = source["model"], source["limits"], source["losses"]
        else:
            source = authored if id_ in HTML_BLOCK_IDS else semantic
            require(id_ in source, "Missing authored expectation")
            want, limits, losses = source[id_], [], []
        validate_model(want)
        checks.append(comparison_check("source-authored", normalize_model(want), normalize_model(actual), limits, losses))
    # A complete passing source check resolves only aggregate annotations.
    complete = any(c["method"] == "source-authored" and c["status"] == "projection-pass"
                   and not c["limits"] and not c["losses"] for c in checks)
    failed = next((c for c in checks if c["status"] != "projection-pass"), None)
    model = {"status": failed["status"] if failed else "projection-pass",
             "comparison": "+".join(c["method"] for c in checks), "checks": checks,
             "limits": [] if complete else sorted({v for c in checks for v in c["limits"]}),
             "losses": [] if complete else sorted({v for c in checks for v in c["losses"]})}
    if failed:
        model.update({k: failed[k] for k in ("expected", "actual", "diff", "reason") if k in failed})
    return model


def validate_response(response, examples):
    keys(response, ("schema", "qt", "cmark", "parseOptions", "htmlOptions", "examples"), "probe response")
    require(type(response["schema"]) is int and response["schema"] == 2, "Invalid probe schema")
    require(isinstance(response["qt"], str) and response["qt"], "Invalid Qt version")
    require(response["cmark"] == "0.31.2", "Unexpected bundled parser")
    require(response["parseOptions"] == PARSE_OPTIONS and response["htmlOptions"] == HTML_OPTIONS, "Unexpected parser options")
    require(isinstance(response["examples"], list) and len(response["examples"]) == len(examples), "Missing probe examples")
    for result, fixture in zip(response["examples"], examples):
        keys(result, ("example", "html", "model"), "probe example")
        require(integer(result["example"]) and result["example"] == fixture["example"], "Invalid probe ID/order")
        require(isinstance(result["html"], str), "Invalid parser HTML")
        validate_model(result["model"])


def run_probe(probe, examples):
    request = {"examples": [{"example": e["example"], "markdown": e["markdown"]} for e in examples]}
    result = subprocess.run([str(Path(probe).resolve())], input=json_text(request), text=True,
                            encoding="utf-8", capture_output=True, timeout=60, check=True)
    response = json.loads(result.stdout)
    validate_response(response, examples)
    return response


def difference(want, actual):
    return "".join(difflib.unified_diff(want.splitlines(keepends=True), actual.splitlines(keepends=True),
                                      fromfile="expected", tofile="actual"))


def normalize_model(blocks):
    from oracle import normalize_inline
    result = []
    for block in blocks:
        value = dict(block)
        if "inlines" in value:
            value["inlines"] = normalize_inline(value["inlines"])
        if "children" in value:
            value["children"] = normalize_model(value["children"])
        result.append(value)
    return result


def analyze(manifest, fixtures, response, authored=None):
    validate_response(response, fixtures)
    authored = load_authored(manifest) if authored is None else authored
    remaining = load_remaining(manifest)
    semantic = load_semantic(manifest)
    results = []
    for fixture, actual in zip(fixtures, response["examples"]):
        parser = {"status": "pass" if fixture["html"] == actual["html"] else "fail"}
        if parser["status"] == "fail":
            parser.update(expected=fixture["html"], actual=actual["html"],
                          diff=difference(fixture["html"], actual["html"]))
        model = model_result(fixture, actual["model"], authored, remaining, semantic)
        results.append({"example": fixture["example"], "section": fixture["section"], "parser": parser, "model": model,
                        "nativeModel": actual["model"]})
    report = {"schema": 4, "commonmark": manifest["version"], "fixtureSha256": manifest["sha256"],
              "tools": {"python": platform.python_version(), "qt": response["qt"], "cmark": response["cmark"]},
              "parseOptions": response["parseOptions"], "htmlOptions": response["htmlOptions"],
              "examples": results, "totals": totals(results), "sections": {}}
    for section in sorted({e["section"] for e in results}):
        report["sections"][section] = totals([e for e in results if e["section"] == section])
    validate_report(report, manifest, fixtures)
    return report


def totals(examples):
    parser = Counter(e["parser"]["status"] for e in examples)
    model = Counter(e["model"]["status"] for e in examples)
    return {"examples": len(examples), "parser": {s: parser[s] for s in ("pass", "fail")},
            "model": {s: model[s] for s in ("projection-pass", "mismatch", "uncheckable")},
            "examplesWithLimits": sum(bool(e["model"]["limits"]) for e in examples),
            "examplesWithLosses": sum(bool(e["model"]["losses"]) for e in examples)}


def validate_report(report, manifest, fixtures):
    keys(report, ("schema", "commonmark", "fixtureSha256", "tools", "parseOptions", "htmlOptions", "examples", "totals", "sections"), "report")
    keys(report["tools"], ("python", "qt", "cmark"), "report tools")
    require(all(isinstance(v, str) and v for v in report["tools"].values()), "Invalid report tools")
    require(report["parseOptions"] == PARSE_OPTIONS and report["htmlOptions"] == HTML_OPTIONS, "Invalid report options")
    require(type(report["schema"]) is int and report["schema"] == 4 and report["commonmark"] == manifest["version"]
            and report["fixtureSha256"] == manifest["sha256"], "Invalid report provenance")
    require(isinstance(report["examples"], list) and len(report["examples"]) == len(fixtures), "Invalid report count")
    remaining = load_remaining(manifest)
    authored = load_authored(manifest)
    semantic = load_semantic(manifest)
    for entry, fixture in zip(report["examples"], fixtures):
        keys(entry, ("example", "section", "parser", "model", "nativeModel"), "report example")
        validate_model(entry["nativeModel"])
        require(integer(entry["example"]) and entry["example"] == fixture["example"] and entry["section"] == fixture["section"], "Invalid report ID/section")
        parser, model = entry["parser"], entry["model"]
        require(isinstance(parser, dict) and isinstance(model, dict), "Invalid report result objects")
        require(parser.get("status") in ("pass", "fail"), "Invalid parser status")
        keys(parser, ("status",) if parser["status"] == "pass" else ("status", "expected", "actual", "diff"), "parser result")
        # Recompute every required check from pinned evidence. This rejects missing
        # checks, fabricated passes, lost HTML annotations and forged aggregation.
        require(model == model_result(fixture, entry["nativeModel"], authored, remaining, semantic),
                "Invalid model checks or aggregate evidence")
        status = model["status"]
        if parser["status"] == "fail":
            require(parser["expected"] == fixture["html"] and isinstance(parser["actual"], str)
                    and parser["actual"] != parser["expected"]
                    and parser["diff"] == difference(parser["expected"], parser["actual"]), "Invalid parser mismatch evidence")
        if status == "mismatch":
            require(isinstance(model["expected"], list) and isinstance(model["actual"], list)
                    and model["expected"] != model["actual"]
                    and model["diff"] == difference(json_text(model["expected"]), json_text(model["actual"])), "Invalid model mismatch evidence")
    require(report["totals"] == totals(report["examples"]), "Invalid report totals")
    sections = {s: totals([e for e in report["examples"] if e["section"] == s]) for s in sorted({e["section"] for e in fixtures})}
    require(report["sections"] == sections, "Invalid section totals")


def exception(entry):
    """Pin exact known mismatch evidence so a worse failure cannot stay green."""
    parser, model = entry["parser"], entry["model"]
    if parser["status"] == "pass" and model["status"] == "projection-pass" and not model["limits"] and not model["losses"]:
        return None
    result = {"parser": parser["status"], "model": model["status"], "limits": model["limits"], "losses": model["losses"],
              "modelSha256": digest(json_text(entry["nativeModel"]).encode())}
    if "reason" in model:
        result["reason"] = model["reason"]
    if parser["status"] == "fail" or model["status"] == "mismatch":
        result["evidenceSha256"] = digest(json_text({"parser": parser, "model": model}).encode())
    return result


def compare_ledger(report, ledger):
    keys(ledger, ("schema", "fixtureSha256", "review", "examples"), "ledger")
    require(type(ledger["schema"]) is int and ledger["schema"] == 1
            and ledger["fixtureSha256"] == report["fixtureSha256"], "Invalid ledger provenance")
    require(isinstance(ledger["review"], str) and ledger["review"], "Missing ledger review")
    require(isinstance(ledger["examples"], dict), "Invalid ledger examples")
    ids = {str(e["example"]) for e in report["examples"]}
    require(set(ledger["examples"]) <= ids, "Unknown ledger example IDs")
    issues = []
    for entry in report["examples"]:
        id_ = str(entry["example"])
        want = ledger["examples"].get(id_)
        actual = exception(entry)
        if want is not None:
            require(isinstance(want, dict), "Invalid ledger entry")
            permitted = {"parser", "model", "limits", "losses", "reason", "modelSha256", "evidenceSha256", "review"}
            require(set(want) <= permitted and {"parser", "model", "limits", "losses", "modelSha256", "review"} <= set(want), "Invalid ledger entry fields")
            require(isinstance(want["review"], str) and want["review"], "Missing per-ID review")
            require(want["parser"] in ("pass", "fail") and want["model"] in ("projection-pass", "mismatch", "uncheckable"), "Invalid ledger status")
            for category in ("limits", "losses"):
                values = want[category]
                require(isinstance(values, list) and all(isinstance(v, str) and v for v in values)
                        and values == sorted(set(values)), "Invalid ledger distinctions")
            require(isinstance(want["modelSha256"], str) and len(want["modelSha256"]) == 64
                    and all(c in "0123456789abcdef" for c in want["modelSha256"]), "Invalid ledger model digest")
            require(("reason" in want) == (want["model"] == "uncheckable"), "Invalid ledger reason")
            if "reason" in want:
                require(isinstance(want["reason"], str) and want["reason"], "Missing ledger reason")
            require(("evidenceSha256" in want) == (want["parser"] == "fail" or want["model"] == "mismatch"), "Invalid ledger mismatch digest")
            if "evidenceSha256" in want:
                require(isinstance(want["evidenceSha256"], str) and len(want["evidenceSha256"]) == 64
                        and all(c in "0123456789abcdef" for c in want["evidenceSha256"]), "Invalid ledger evidence digest")
            want = {k: v for k, v in want.items() if k != "review"}
        if actual != want:
            issues.append({"example": entry["example"], "expected": want, "actual": actual})
    return issues


def strict_pass(report):
    total = report["totals"]
    return not (total["parser"]["fail"] or total["model"]["mismatch"] or total["model"]["uncheckable"]
                or total["examplesWithLimits"] or total["examplesWithLosses"])
