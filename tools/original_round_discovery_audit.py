#!/usr/bin/env python3
"""Bounded original-resource discovery and continuous-round replay audit.

Analysis/CI only. Original Windows v1.014 metadata and licensed assets must
be sourced and verified by the enclosing CI workflow. The solver only selects
shot inputs; all flight, landing, score and game flow use the unchanged C++ core.
"""
from __future__ import annotations

import argparse
import json
import struct
import subprocess
import sys
from pathlib import Path


def parse_fixture(path: Path) -> list[dict]:
    rows = []
    for line_number, line in enumerate(path.read_text().splitlines(), 1):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = list(map(int, line.split()))
        if len(parts) < 7 or len(parts) != 7 + 4 * parts[6]:
            raise ValueError(f"bad fixture line {line_number}")
        shots = [
            {"aim": parts[n], "power": parts[n + 1],
             "accuracy": parts[n + 2], "club": parts[n + 3]}
            for n in range(7, len(parts), 4)
        ]
        rows.append({
            "id": parts[0], "par": parts[1],
            "tee": parts[2:4], "cup": parts[4:6],
            "shots": shots,
        })
    return rows


def spt_coordinates(root: Path, rid: int) -> tuple[list[int], list[int]]:
    path = root / f"MAPM{rid:02d}.SPT"
    data = path.read_bytes()
    if len(data) != 50:
        raise ValueError(f"{path} has unexpected SPT size {len(data)}")
    values = struct.unpack(">25H", data)
    # Original per-player slot 0 and original hole/cup slot 4, words 2/3.
    return list(values[2:4]), list(values[22:24])


def fixture_text(rows: list[dict]) -> str:
    lines = [
        "# Windows v1.014 original slot-0 numeric replay (test-only).",
        "# id par tee_x tee_y cup_x cup_y nshots [aim power accuracy club]...",
    ]
    for row in rows:
        cells = [row["id"], row["par"], *row["tee"], *row["cup"],
                 len(row["shots"])]
        for shot in row["shots"]:
            cells.extend([shot["aim"], shot["power"],
                          shot["accuracy"], shot["club"]])
        lines.append(" ".join(str(x) for x in cells))
    return "\n".join(lines) + "\n"


def verify_prefix(rows: list[dict], order: list[int], pars: list[int],
                  root: Path) -> None:
    if len(rows) < 2 or len(rows) > 18:
        raise ValueError("prefix must contain 2-18 actual course rows")
    for i, row in enumerate(rows):
        if row["id"] != order[i] or row["par"] != pars[row["id"]]:
            raise ValueError(f"prefix hole {i} disagrees with original binary")
        tee, cup = spt_coordinates(root, row["id"])
        if row["tee"] != tee or row["cup"] != cup:
            raise ValueError(f"prefix hole {i} SPT tee/cup mismatch")
        if i < len(rows)-1 and not row["shots"]:
            raise ValueError(f"prefix hole {i} is missing verified shots")
    if rows[-1]["shots"]:
        raise ValueError("prefix must end with one unplayed original course")


def run_original_solver(solver: Path, root: Path, rid: int, par: int,
                        next_id: int, next_par: int, out: Path) -> dict:
    current = f"{rid:02d}"
    nxt = f"{next_id:02d}"
    command = [
        str(solver), str(rid), str(par), str(next_id), str(next_par),
        str(root / f"MAPM{current}.MAP"),
        str(root / f"MAPS{current}.MAP"),
        str(root / f"MAPM{current}.SPT"),
        str(root / "MAPI01.RAW"), str(root / "MAPI02.RAW"),
        str(root / f"MAPM{nxt}.MAP"),
        str(root / f"MAPS{nxt}.MAP"),
        str(root / f"MAPM{nxt}.SPT"), "0",
    ]
    process = subprocess.run(command, text=True, capture_output=True,
                             timeout=180, check=False)
    (out / f"solver_{rid:02d}.stderr.txt").write_text(process.stderr)
    (out / f"solver_{rid:02d}.stdout.txt").write_text(process.stdout)
    if process.returncode != 0:
        raise RuntimeError(
            f"original resource {rid} solver exited {process.returncode}; "
            f"see retained per-hole diagnostic logs")
    data = json.loads(process.stdout)
    if (data.get("resource_id") != rid or data.get("par") != par
        or data.get("next_resource_id") != next_id
        or not data.get("next_loaded")
        or not data.get("shots")
        or data.get("strokes") != len(data["shots"])):
        raise ValueError(f"resource {rid} solver replay failed strict contract")
    (out / f"hole_{rid:02d}.json").write_text(
        json.dumps(data, indent=2) + "\n")
    return data


def run_continuous_replay(replay: Path, root: Path, fixture: Path,
                          out: Path, completed: int) -> dict:
    process = subprocess.run(
        [str(replay), str(root), str(fixture), "0"],
        text=True, capture_output=True, timeout=120, check=False)
    (out / "continuous.stderr.txt").write_text(process.stderr)
    (out / "continuous.stdout.txt").write_text(process.stdout)
    if process.returncode:
        raise RuntimeError(
            f"continuous {completed}-hole replay failed "
            f"exit={process.returncode}")
    value = json.loads(process.stdout)
    if value["holes_completed"] != completed or (
        completed < 18 and not value["next_loaded"]) or (
        completed == 18 and not value["round_complete"]):
        raise ValueError(f"wrong {completed}-hole continuous round result: {value}")
    (out / "continuous_result.json").write_text(
        json.dumps(value, indent=2) + "\n")
    return value


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--original-metadata", type=Path, required=True)
    ap.add_argument("--original-root", type=Path, required=True)
    ap.add_argument("--prefix-fixture", type=Path, required=True)
    ap.add_argument("--solver", type=Path, required=True)
    ap.add_argument("--replay", type=Path, required=True)
    ap.add_argument("--output", type=Path, required=True)
    ap.add_argument("--max-new", type=int, default=10)
    args = ap.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)

    metadata = json.loads(args.original_metadata.read_text())
    order = next(x["hole_ids"] for x in metadata["orders"] if x["slot"] == 0)
    pars = metadata["par_by_resource_id"]
    if len(order) != 18 or any(i < 0 or i >= 100 for i in order):
        raise ValueError("unexpected original v1.014 course order")
    rows = parse_fixture(args.prefix_fixture)
    verify_prefix(rows, order, pars, args.original_root)
    if not 1 <= args.max_new <= 18:
        raise ValueError("max-new must be between 1 and 18")

    fixture = args.output / "verified_original_round_fixture.txt"
    fixture.write_text(fixture_text(rows))
    errors = []
    start = len(rows) - 1

    for index in range(start, min(18, start + args.max_new)):
        rid = order[index]
        next_id = order[index + 1] if index + 1 < 18 else order[0]
        next_par = pars[next_id]
        par = pars[rid]
        print(f"original-round discovery index={index} resource={rid} "
              f"par={par} next={next_id}", flush=True)
        try:
            result = run_original_solver(
                args.solver, args.original_root, rid, par,
                next_id, next_par, args.output)
            if rows[-1]["id"] != rid:
                raise ValueError("discovery order/fixture divergence")
            rows[-1]["shots"] = result["shots"]
            if index + 1 < 18:
                tee, cup = spt_coordinates(args.original_root, next_id)
                if [result["next_tee_x"],result["next_tee_y"]] != tee or (
                    [result["next_cup_x"],result["next_cup_y"]] != cup):
                    raise ValueError("solver next-course SPT metadata mismatch")
                rows.append({"id": next_id, "par": next_par,
                             "tee": tee, "cup": cup, "shots": []})
            fixture.write_text(fixture_text(rows))
            verify_result = run_continuous_replay(
                args.replay, args.original_root, fixture,
                args.output, index + 1)
            print(f"  verified original holes={index+1} "
                  f"strokes={verify_result['total_strokes']} "
                  f"original-par-minus-strokes={verify_result['relative_to_par']}",
                  flush=True)
        except Exception as exc:
            message = f"index {index} resource {rid}: {exc}"
            errors.append(message)
            print("BOUNDARY: " + message, file=sys.stderr, flush=True)
            break

    report = {
        "reference": "Original verified Windows v1.014",
        "order": order,
        "requested_max_new": args.max_new,
        "initial_completed": start,
        "now_completed": len(rows) if rows[-1]["shots"] else len(rows)-1,
        "round_complete": bool(rows[-1]["shots"] and len(rows)==18),
        "failure": errors,
        "fixture": fixture.name,
        "commercial_assets_in_repo": False,
    }
    (args.output / "report.json").write_text(json.dumps(report, indent=2)+"\n")
    print(json.dumps(report, indent=2),flush=True)
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
