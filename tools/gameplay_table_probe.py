#!/usr/bin/env python3
"""Probe static gameplay tables from the verified Sensible Golf Windows PE.

Outputs compact derived metadata only; it does not copy the executable.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
from pathlib import Path

from executable_probe import parse_executable


class ProbeError(ValueError):
    pass


def va_to_file_offset(probe: dict, va: int) -> int:
    image_base=probe.get("image_base")
    if image_base is None:
        raise ProbeError("PE image base missing")
    rva=va-int(image_base)
    for section in probe.get("sections",[]):
        start=int(section["virtual_address"])
        raw_size=int(section["raw_size"])
        if start <= rva < start+raw_size:
            return int(section["raw_offset"])+(rva-start)
    raise ProbeError(f"VA 0x{va:x} is not backed by raw PE data")


def read_va(data: bytes, probe: dict, va: int, size: int)->bytes:
    off=va_to_file_offset(probe,va)
    chunk=data[off:off+size]
    if len(chunk)!=size:
        raise ProbeError(f"truncated read at VA 0x{va:x}")
    return chunk


def s16s(data: bytes)->list[int]:
    if len(data)%2:
        raise ProbeError("signed-16 data length must be even")
    return list(struct.unpack("<"+"h"*(len(data)//2),data))


def s32s(data: bytes)->list[int]:
    if len(data)%4:
        raise ProbeError("signed-32 data length must be divisible by four")
    return list(struct.unpack("<"+"i"*(len(data)//4),data))


def analyse_trig(values:list[int])->dict[str,object]:
    if len(values)<5120:
        raise ProbeError("expected at least 5120 trig words")

    cycle=values[:4096]
    phase=values[1024:5120]
    phase_identical=(phase==values[1024:5120])

    samples={str(i):values[i] for i in [0,256,512,768,1024,1536,2048,2560,3072,3584,4095,4096,5119]}

    # Test common exact-generation hypotheses. This is diagnostic; equality,
    # not visual similarity, is what matters.
    candidates={}
    for func_name,fn in (
        ("sin",math.sin),
        ("cos",math.cos),
    ):
        for scale in (16383,16384):
            for method_name,convert in (
                ("round",round),
                ("trunc",int),
            ):
                expected=[
                    int(convert(scale*fn(2*math.pi*i/4096)))
                    for i in range(4096)
                ]
                diffs=[abs(a-b) for a,b in zip(cycle,expected)]
                candidates[f"{func_name}_{scale}_{method_name}"]={
                    "max_abs_error":max(diffs),
                    "mean_abs_error":sum(diffs)/len(diffs),
                    "exact_count":sum(d==0 for d in diffs),
                }

    best=min(candidates.items(),key=lambda kv:(kv[1]["max_abs_error"],kv[1]["mean_abs_error"]))

    return {
        "word_count":len(values),
        "cycle_words":4096,
        "quarter_cycle_words":1024,
        "min":min(cycle),
        "max":max(cycle),
        "samples":samples,
        "second_lookup_is_same_table_plus_1024_words":phase_identical,
        "generation_candidates":candidates,
        "best_generation_candidate":{"name":best[0],**best[1]},
        "cycle_sha256":hashlib.sha256(struct.pack("<"+"h"*4096,*cycle)).hexdigest(),
    }


def analyse(executable:Path)->dict[str,object]:
    data=executable.read_bytes()
    probe=parse_executable(data)
    if probe.get("format")!="PE":
        raise ProbeError("requires PE executable")

    trig_base=0x41A670
    trig_values=s16s(read_va(data,probe,trig_base,5120*2))

    mapping_base=0x41D4C4
    mapping=list(read_va(data,probe,mapping_base,160))
    # Mapping values are 1-based record ids where non-zero.
    nonzero=[x for x in mapping if x]
    max_record=max(nonzero) if nonzero else 0

    records_base=0x41F204
    record_count=max(max_record,16)
    record_bytes=read_va(data,probe,records_base,record_count*0x1C)
    records=[]
    for i in range(record_count):
        raw=record_bytes[i*0x1C:(i+1)*0x1C]
        records.append({
            "record":i+1,
            "hex":raw.hex(),
            "s16":s16s(raw),
            "s32":s32s(raw),
        })

    thresholds=struct.unpack("<HH",read_va(data,probe,0x41D4C0,4))

    return {
        "executable":executable.name,
        "sha256":probe["sha256"],
        "trig":{
            "base_va":"0x0041a670",
            "phase_lookup_va":"0x0041ae70",
            **analyse_trig(trig_values),
        },
        "accuracy":{
            "lower_threshold":thresholds[0],
            "upper_threshold":thresholds[1],
            "centre_constant":0x3F,
        },
        "launch_mapping":{
            "base_va":"0x0041d4c4",
            "bytes_examined":len(mapping),
            "rows_of_10":[mapping[i:i+10] for i in range(0,len(mapping),10)],
            "nonzero_record_ids":sorted(set(nonzero)),
            "max_record_id":max_record,
        },
        "launch_records":{
            "base_va":"0x0041f204",
            "record_stride":0x1C,
            "records":records,
        },
    }


def main()->int:
    parser=argparse.ArgumentParser(description="Probe Sensible Golf static gameplay tables")
    parser.add_argument("executable",type=Path)
    parser.add_argument("-o","--output",type=Path)
    args=parser.parse_args()
    try:
        report=analyse(args.executable)
    except (OSError,ProbeError,struct.error) as exc:
        parser.error(str(exc))
    text=json.dumps(report,indent=2)+"\n"
    if args.output:
        args.output.write_text(text,encoding="utf-8")
    else:
        print(text,end="")
    return 0


if __name__=="__main__":
    raise SystemExit(main())
