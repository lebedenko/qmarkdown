#!/usr/bin/env python3
"""Read the explicit CI Qt targets (standard library only)."""

import argparse
import json
from pathlib import Path
import re

CONFIG = Path(__file__).with_name("qt-versions.json")
ROLES = ("minimum", "development")


def load_versions(path=CONFIG):
    values = json.loads(Path(path).read_text())
    if not isinstance(values, dict) or set(values) != set(ROLES):
        raise ValueError("Qt configuration must contain only minimum and development")
    for version in values.values():
        if not isinstance(version, str) or not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", version):
            raise ValueError("Qt targets must be exact numeric X.Y.Z versions")
    ordered = [tuple(map(int, values[role].split("."))) for role in ROLES]
    if ordered[0] >= ordered[1]:
        raise ValueError("Development Qt must be newer than minimum Qt")
    return values


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    selection = parser.add_mutually_exclusive_group(required=True)
    selection.add_argument("--role", choices=ROLES)
    selection.add_argument("--validate-version")
    selection.add_argument("--matrix", action="store_true")
    args = parser.parse_args()
    try:
        versions = load_versions()
        if args.role:
            print(versions[args.role])
        elif args.matrix:
            print(json.dumps({"include": [{"role": role, "qt": versions[role]} for role in ROLES]}))
        elif args.validate_version not in versions.values():
            parser.error("Qt version is not a configured target")
    except (OSError, ValueError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()
