#!/usr/bin/env python3
"""Run the pinned offline CommonMark baseline; --strict demands complete coverage."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tests/commonmark"))
from baseline import DATA, analyze, compare_ledger, json_text, load_fixtures, run_probe, strict_pass


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", type=Path, default=Path("build-shared/tests/qmarkdown-commonmark-probe"))
    parser.add_argument("--report", type=Path, default=Path("build-shared/tests/commonmark-report.json"))
    parser.add_argument("--strict", action="store_true")
    args = parser.parse_args()
    try:
        manifest, fixtures = load_fixtures()
        response = run_probe(args.probe, fixtures)
        report = analyze(manifest, fixtures, response)
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json_text(report), encoding="utf-8")
        ledger = json.loads((DATA / "ledger.json").read_text())
        changes = compare_ledger(report, ledger)
        t = report["totals"]
        print(f"CommonMark {report['commonmark']}: {t['examples']} examples; parser {t['parser']}; model {t['model']}")
        print(f"Model evidence: {t['examplesWithLimits']} examples with comparison limits; {t['examplesWithLosses']} with semantic information loss.")
        for entry in report["examples"]:
            if entry["parser"]["status"] == "fail":
                print(f"Parser mismatch {entry['example']} ({entry['section']}):\n{entry['parser']['diff']}")
            if entry["model"]["status"] == "mismatch":
                print(f"Model mismatch {entry['example']} ({entry['section']}):\n{entry['model']['diff']}")
        print(f"Report: {args.report}")
        for change in changes:
            print(f"Unexpected baseline change: {json.dumps(change, sort_keys=True)}", file=sys.stderr)
        if changes:
            return 1
        if args.strict and not strict_pass(report):
            print("FAIL: strict conformance requires no failures, uncheckable examples, unresolved comparison limits or semantic loss.", file=sys.stderr)
            return 1
        print("PASS: reviewed baseline unchanged; full CommonMark conformance is not established.")
        return 0
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        print(f"CommonMark baseline error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
