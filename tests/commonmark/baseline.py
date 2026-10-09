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


def validate_model(blocks, context="root"):
    require(isinstance(blocks, list), "Invalid block array")
    fields = {
        "Paragraph": ("kind", "text", "ranges", "links", "images"),
        "Heading": ("kind", "level", "text", "ranges", "links", "images"),
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


def validate_authored(value, manifest, ids=HTML_BLOCK_IDS, annotations=False):
    keys(value, ("schema", "fixtureSha256", "review", "examples"), "authored expectations")
    require(type(value["schema"]) is int and value["schema"] == 1
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


def validate_response(response, examples):
    keys(response, ("schema", "qt", "cmark", "parseOptions", "htmlOptions", "examples"), "probe response")
    require(type(response["schema"]) is int and response["schema"] == 1, "Invalid probe schema")
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


def analyze(manifest, fixtures, response, authored=None):
    validate_response(response, fixtures)
    authored = load_authored(manifest) if authored is None else authored
    remaining = load_remaining(manifest)
    results = []
    for fixture, actual in zip(fixtures, response["examples"]):
        parser = {"status": "pass" if fixture["html"] == actual["html"] else "fail"}
        if parser["status"] == "fail":
            parser.update(expected=fixture["html"], actual=actual["html"],
                          diff=difference(fixture["html"], actual["html"]))
        try:
            source_authored = fixture["example"] in AUTHORED_IDS
            if source_authored:
                if fixture["example"] in REMAINING_IDS:
                    entry = remaining[fixture["example"]]
                    want, limits, losses = entry["model"], entry["limits"], entry["losses"]
                else:
                    require(fixture["example"] in authored, "Missing authored expectation")
                    want, limits, losses = authored[fixture["example"]], [], []
                validate_model(want)
                projection = actual["model"]
            else:
                want, limits, losses = expected(fixture["html"])
                projection = canonical_actual(actual["model"])
            model = {"status": "projection-pass" if want == projection else "mismatch",
                     "limits": limits, "losses": losses}
            if model["status"] == "mismatch":
                model.update(expected=want, actual=projection, diff=difference(json_text(want), json_text(projection)))
        except Uncheckable as error:
            model = {"status": "uncheckable", "reason": str(error), "limits": ["html-oracle-unsupported"], "losses": []}
        model["comparison"] = "source-authored" if fixture["example"] in AUTHORED_IDS else "html-projection"
        results.append({"example": fixture["example"], "section": fixture["section"], "parser": parser, "model": model,
                        "nativeModel": actual["model"]})
    report = {"schema": 2, "commonmark": manifest["version"], "fixtureSha256": manifest["sha256"],
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
    require(type(report["schema"]) is int and report["schema"] == 2 and report["commonmark"] == manifest["version"]
            and report["fixtureSha256"] == manifest["sha256"], "Invalid report provenance")
    require(isinstance(report["examples"], list) and len(report["examples"]) == len(fixtures), "Invalid report count")
    remaining = load_remaining(manifest)
    for entry, fixture in zip(report["examples"], fixtures):
        keys(entry, ("example", "section", "parser", "model", "nativeModel"), "report example")
        validate_model(entry["nativeModel"])
        require(integer(entry["example"]) and entry["example"] == fixture["example"] and entry["section"] == fixture["section"], "Invalid report ID/section")
        parser, model = entry["parser"], entry["model"]
        require(isinstance(parser, dict) and isinstance(model, dict), "Invalid report result objects")
        require(parser.get("status") in ("pass", "fail"), "Invalid parser status")
        keys(parser, ("status",) if parser["status"] == "pass" else ("status", "expected", "actual", "diff"), "parser result")
        status = model.get("status")
        require(status in ("projection-pass", "mismatch", "uncheckable"), "Invalid model status")
        keys(model, ("status", "comparison", "limits", "losses", *(("expected", "actual", "diff") if status == "mismatch"
             else ("reason",) if status == "uncheckable" else ())), "model result")
        require(model["comparison"] == ("source-authored" if fixture["example"] in AUTHORED_IDS else "html-projection"), "Invalid comparison method")
        if model["comparison"] == "source-authored":
            annotations = remaining.get(fixture["example"], {"limits": [], "losses": []})
            require(status != "uncheckable" and all(model[k] == annotations[k] for k in ("limits", "losses")),
                    "Invalid authored evidence limits/losses")
        for category in ("limits", "losses"):
            values = model[category]
            require(isinstance(values, list) and all(isinstance(v, str) and v for v in values)
                    and values == sorted(set(values)), f"Invalid report {category}")
        if status == "uncheckable":
            require(isinstance(model["reason"], str) and model["reason"], "Missing uncheckable reason")
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
