#!/usr/bin/env python3
"""Machine-verify original v1.014 terminal code-35/code-50 branch.

Execute source Windows x86 0x40A686 through its FIRST external branch.
No attempt is made to infer whether code 50 denotes a tree, obstacle or
other real-course object. This is an isolated route, NOT end-to-end impact.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import random
import struct
import subprocess
import sys
from itertools import product
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from original_v1014_oracle import build_uc, REFERENCE_SHA256, BALL
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EDI, UC_X86_REG_EBX

START = 0x40A686
TARGETS = {0x40A730: 0, 0x40A6E3: 1, 0x40A6AB: 2}

def original(uc, row):
    landing, height = row
    uc.mem_write(BALL + 0x16, struct.pack("<H", height & 0xFFFF))
    uc.reg_write(UC_X86_REG_EDI, BALL)
    uc.reg_write(UC_X86_REG_EBX, landing & 0xFFFF)
    reached = []
    def hook(machine, address, size, user):
        if address in TARGETS:
            reached.append(address)
            machine.emu_stop()
    token = uc.hook_add(UC_HOOK_CODE, hook)
    try:
        uc.emu_start(START, 0, count=40)
    finally:
        uc.hook_del(token)
    if len(reached) != 1:
        raise RuntimeError(f"unresolved terminal x86 route {row}: {reached}")
    return TARGETS[reached[0]], hex(reached[0])

def test_vectors():
    routes = (0, 1, 6, 8, 9, 10, 35, 36, 49, 50, 51, 255, 65535)
    heights = (-32768, -32767, -1, 0, 1, 199, 200, 201, 202, 32767)
    rows = list(product(routes, heights))
    rnd = random.Random(1014)
    for _ in range(2600):
        rows.append((rnd.choice((35, 35, 50, 50, 8, 9, 51, rnd.randrange(65536))),
                     rnd.choice((199, 200, 201, -1, -32768, rnd.randint(-32768, 32767)))))
    return rows

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--exe", type=Path, required=True)
    p.add_argument("--probe", type=Path, required=True)
    p.add_argument("-o", "--output", type=Path, required=True)
    a = p.parse_args()
    if hashlib.sha256(a.exe.read_bytes()).hexdigest() != REFERENCE_SHA256:
        raise SystemExit("original Windows v1.014 executable checksum mismatch")
    vectors = test_vectors()
    process = subprocess.run([str(a.probe)],
        input="".join(f"{landing} {height}\n" for landing,height in vectors),
        text=True, capture_output=True, check=True)
    outputs = process.stdout.splitlines()
    if len(outputs) != len(vectors):
        raise RuntimeError("native route probe returned wrong number of rows")
    uc, _ = build_uc(a.exe)
    bad, targets = [], {}
    for row, output in zip(vectors, outputs):
        exact, target = original(uc, row)
        targets[target] = targets.get(target, 0) + 1
        got = int(output)
        if got != exact:
            bad.append({"case": row, "original": exact, "native": got, "target": target})
    evidence = {
        "schema": "pixtee.original-terminal-code35-code50-branch-x86.v1",
        "original_sha256": REFERENCE_SHA256,
        "case_count": len(vectors),
        "exact_matches": len(vectors) - len(bad),
        "original_target_counts": targets,
        "mismatches": bad[:80],
        "scope": "Original 0x40A686 branch only. No object identity, course presence, tree physics, audio, animation or complete collision proven."
    }
    a.output.parent.mkdir(parents=True, exist_ok=True)
    a.output.write_text(json.dumps(evidence, indent=2) + "\n")
    print("Exact original/native event routing:",
          evidence["exact_matches"], "/", len(vectors))
    print("Reached authentic x86 targets:", targets)
    if bad:
        for item in bad[:12]:
            print("MISMATCH", item)
        raise SystemExit("original terminal event branch mismatch")
    print("PASS original x86 branch classification, not game-wide tree parity")

if __name__ == "__main__":
    main()
