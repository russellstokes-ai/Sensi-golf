#!/usr/bin/env python3
"""Prepare a personal full-original Sensible Golf DOS game archive.

The game binaries MUST be supplied by the operator. Nothing is downloaded,
and no game content is stored in the source repository. This is for personal
proof-of-concept compatibility testing, not a claim of distribution rights.
"""
from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import zipfile

KNOWN_HASHES = {
    "GOLFDOS.EXE": "14c049b2456cda9bc7f54ce1c999fac815879775baf2807a76ef889559d0e1ed",
    "GOLF.EPF": "58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e",
}
NON_DOS_EXE = {"GOLFWIN.EXE"}
MAX_SINGLE_FILE = 40 * 1024 * 1024
MAX_TOTAL_FILE_BYTES = 80 * 1024 * 1024

def collect(entries) -> dict[str, bytes]:
    records = {}
    total = 0
    for pathname, data in entries:
        name = Path(pathname.replace("\\", "/")).name.upper()
        if not name or name in {".", ".."}: continue
        if "/" in name or "\\" in name: raise ValueError("invalid flat game file path")
        if name in NON_DOS_EXE: continue
        if name.endswith((".EXE", ".COM", ".BAT")) and name != "GOLFDOS.EXE": continue
        if len(data) > MAX_SINGLE_FILE: raise ValueError("oversize file " + name)
        total += len(data)
        if total > MAX_TOTAL_FILE_BYTES: raise ValueError("game source too large")
        if name in records and records[name] != data: raise ValueError("duplicate name " + name)
        records[name] = data
    missing = set(KNOWN_HASHES) - records.keys()
    if missing: raise ValueError("full original DOS game files missing: " + ", ".join(sorted(missing)))
    for name, hash_ in KNOWN_HASHES.items():
        actual = hashlib.sha256(records[name]).hexdigest()
        if actual != hash_:
            raise ValueError(f"{name} is not the validated DOS v1.014 reference: {actual}")
    return records

def load(source: Path) -> dict[str, bytes]:
    if source.is_dir():
        return collect((str(p.relative_to(source)), p.read_bytes())
                       for p in sorted(source.rglob("*")) if p.is_file())
    if source.is_file() and zipfile.is_zipfile(source):
        with zipfile.ZipFile(source) as archive:
            candidates = [item for item in archive.infolist() if not item.is_dir()]
            if len(candidates)>1000: raise ValueError("too many archive entries")
            for i in candidates:
                if i.file_size > MAX_SINGLE_FILE: raise ValueError("oversize ZIP entry")
            return collect((item.filename, archive.read(item)) for item in candidates)
    raise ValueError("Pass an extracted original game folder or original ZIP archive")

def pack(source: Path, dest: Path) -> dict[str, str]:
    files = load(source)
    dest.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(dest,"w",compression=zipfile.ZIP_DEFLATED,
                         compresslevel=6,strict_timestamps=False) as z:
        for name in sorted(files):
            info=zipfile.ZipInfo(name,date_time=(1980,1,1,0,0,0))
            info.compress_type=zipfile.ZIP_DEFLATED
            info.external_attr=0o644<<16
            z.writestr(info,files[name],compress_type=zipfile.ZIP_DEFLATED)
    with zipfile.ZipFile(dest) as z:
        executable=[x for x in z.namelist() if x.endswith((".EXE",".COM",".BAT"))]
        assert executable==["GOLFDOS.EXE"],executable
    return {k:hashlib.sha256(v).hexdigest() for k,v in files.items()}

def main() -> int:
    parser=argparse.ArgumentParser(description="Create original DOSBox Pure game archive for private POC")
    parser.add_argument("original_copy",type=Path)
    parser.add_argument("-o","--output",type=Path,
        default=Path("android-full/app/src/main/assets/game/SensibleGolf.dosz"))
    args=parser.parse_args()
    try:
        file_hashes=pack(args.original_copy,args.output)
    except (ValueError,OSError,zipfile.BadZipFile) as exc:
        parser.error(str(exc))
    print(f"Private, gitignored game archive created: {args.output}")
    print(f"Validated {len(file_hashes)} original support files; only DOS executable is GOLFDOS.EXE.")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
