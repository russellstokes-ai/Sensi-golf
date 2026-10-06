#!/usr/bin/env python3
"""Compare an original-game shot trace with a recovered-core trace."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path


class TraceError(ValueError):
    pass


def _point(obj: dict, key: str) -> tuple[float, float]:
    try:
        value = obj[key]
        return float(value["x"]), float(value["y"])
    except (KeyError, TypeError, ValueError) as exc:
        raise TraceError(f"missing/invalid {key} point") from exc


def validate_trace(trace: dict) -> None:
    if trace.get("version") != 1:
        raise TraceError("trace version must be 1")
    for key in ("source", "input", "samples", "events"):
        if key not in trace:
            raise TraceError(f"missing {key}")
    if not isinstance(trace["samples"], list):
        raise TraceError("samples must be a list")

    previous = -1
    seen = set()
    for sample in trace["samples"]:
        for key in ("tick", "x", "y"):
            if key not in sample:
                raise TraceError(f"sample missing {key}")
        tick = sample["tick"]
        if not isinstance(tick, int) or tick < 0:
            raise TraceError("sample tick must be a non-negative integer")
        if tick in seen or tick <= previous:
            raise TraceError("sample ticks must be unique and strictly increasing")
        seen.add(tick)
        previous = tick
        float(sample["x"])
        float(sample["y"])
        if "z" in sample:
            float(sample["z"])

    _point(trace["events"], "landing")
    _point(trace["events"], "rest")


def distance(a: tuple[float, float], b: tuple[float, float]) -> float:
    return math.hypot(a[0] - b[0], a[1] - b[1])


def compare(reference: dict, candidate: dict, xy_tolerance: float, event_tolerance: float) -> dict:
    validate_trace(reference)
    validate_trace(candidate)

    ref_by_tick = {s["tick"]: s for s in reference["samples"]}
    cand_by_tick = {s["tick"]: s for s in candidate["samples"]}
    common = sorted(set(ref_by_tick) & set(cand_by_tick))
    if not common:
        raise TraceError("traces have no common sample ticks")

    errors = []
    z_errors = []
    for tick in common:
        a = ref_by_tick[tick]
        b = cand_by_tick[tick]
        errors.append(math.hypot(float(a["x"]) - float(b["x"]), float(a["y"]) - float(b["y"])))
        if "z" in a and "z" in b:
            z_errors.append(abs(float(a["z"]) - float(b["z"])))

    landing_error = distance(_point(reference["events"], "landing"), _point(candidate["events"], "landing"))
    rest_error = distance(_point(reference["events"], "rest"), _point(candidate["events"], "rest"))

    hazard_match = reference["events"].get("hazard") == candidate["events"].get("hazard")
    holed_match = reference["events"].get("holed") == candidate["events"].get("holed")

    rms_xy = math.sqrt(sum(e * e for e in errors) / len(errors))
    maximum_xy = max(errors)
    missing_reference_ticks = sorted(set(ref_by_tick) - set(cand_by_tick))
    extra_candidate_ticks = sorted(set(cand_by_tick) - set(ref_by_tick))

    passed = (
        maximum_xy <= xy_tolerance
        and landing_error <= event_tolerance
        and rest_error <= event_tolerance
        and hazard_match
        and holed_match
        and not missing_reference_ticks
        and not extra_candidate_ticks
    )

    return {
        "pass": passed,
        "sample_count": len(common),
        "max_xy_error": maximum_xy,
        "rms_xy_error": rms_xy,
        "max_z_error": max(z_errors) if z_errors else None,
        "landing_error": landing_error,
        "rest_error": rest_error,
        "hazard_match": hazard_match,
        "holed_match": holed_match,
        "missing_reference_ticks": missing_reference_ticks,
        "extra_candidate_ticks": extra_candidate_ticks,
        "tolerances": {
            "xy": xy_tolerance,
            "events": event_tolerance,
        },
    }


def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare original and recovered shot traces")
    parser.add_argument("reference", type=Path)
    parser.add_argument("candidate", type=Path)
    parser.add_argument("--xy-tolerance", type=float, required=True)
    parser.add_argument("--event-tolerance", type=float, required=True)
    parser.add_argument("-o", "--output", type=Path)
    args = parser.parse_args()

    try:
        report = compare(
            load(args.reference),
            load(args.candidate),
            args.xy_tolerance,
            args.event_tolerance,
        )
    except (OSError, json.JSONDecodeError, TraceError, TypeError, ValueError) as exc:
        parser.error(str(exc))

    text = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0 if report["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
