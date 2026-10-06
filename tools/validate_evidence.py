#!/usr/bin/env python3
"""Validate the minimum evidence discipline for recovered gameplay claims."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


ALLOWED_STATUS = {"hypothesis", "observed", "cross-checked", "parity-verified"}
ALLOWED_KINDS = {
    "manual",
    "dos-executable",
    "windows-executable",
    "epf-data",
    "black-box-trace",
}


class EvidenceError(ValueError):
    pass


def validate(record: dict) -> None:
    for key in ("id", "status", "claim", "sources", "tests"):
        if key not in record:
            raise EvidenceError(f"missing {key}")

    if not str(record["id"]).strip():
        raise EvidenceError("id must not be empty")
    if not str(record["claim"]).strip():
        raise EvidenceError("claim must not be empty")
    if record["status"] not in ALLOWED_STATUS:
        raise EvidenceError("invalid status")
    if not isinstance(record["sources"], list) or not record["sources"]:
        raise EvidenceError("sources must be a non-empty list")
    if not isinstance(record["tests"], list):
        raise EvidenceError("tests must be a list")

    kinds = set()
    for source in record["sources"]:
        if source.get("kind") not in ALLOWED_KINDS:
            raise EvidenceError("invalid source kind")
        if not str(source.get("source", "")).strip():
            raise EvidenceError("source name must not be empty")
        if source.get("offset") is not None and (
            not isinstance(source["offset"], int) or source["offset"] < 0
        ):
            raise EvidenceError("offset must be a non-negative integer or null")
        kinds.add(source["kind"])

    if record["status"] == "cross-checked":
        executable_kinds = kinds & {"dos-executable", "windows-executable"}
        if len(executable_kinds) < 2 and "black-box-trace" not in kinds:
            raise EvidenceError(
                "cross-checked evidence needs both executable ports or a black-box trace"
            )

    if record["status"] == "parity-verified":
        if "black-box-trace" not in kinds:
            raise EvidenceError("parity-verified evidence requires a black-box trace")
        if not record["tests"]:
            raise EvidenceError("parity-verified evidence requires a named parity test")


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate a recovery evidence JSON record")
    parser.add_argument("record", type=Path)
    args = parser.parse_args()

    try:
        record = json.loads(args.record.read_text(encoding="utf-8"))
        validate(record)
    except (OSError, json.JSONDecodeError, EvidenceError) as exc:
        parser.error(str(exc))

    print(f"valid evidence record: {record['id']} [{record['status']}]")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
