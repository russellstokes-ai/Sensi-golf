#!/usr/bin/env python3
"""Audit original Sensible Golf v1.014 sprite FRAME dimensions from extracted EPF.

Reads actual MCH/ILBM game resources and emits dimensions/hashes ONLY.
Never emits pixel arrays, source sprites, palettes or original art.
MCH decoding follows the documented East Point MCH frame-table format.
Fail closed if the game uses a different format.
"""
from __future__ import annotations

import argparse
import collections
import hashlib
import json
from pathlib import Path
import struct

EXPECTED_EPF_SHA = "58955f2ef89ad1757998b3ceee661a8ab7edcbd88e00bc1c6683009aa123ab1e"

def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def parse_mch(raw: bytes) -> dict:
    if len(raw) < 2:
        return {"status":"unsupported-or-truncated","reason":"too short"}
    count = struct.unpack_from("<H", raw, 0)[0]
    table_end = 2 + 8 * count
    if not 1 <= count <= 5000 or table_end > len(raw):
        return {"status":"unsupported-format","reason":"invalid 16-bit LE frame count/table","candidate_le_count":count}
    records=[]
    invalid=[]
    for i in range(count):
        width,height,offset=struct.unpack_from("<HHI",raw,2+8*i)
        if width==0 or height==0 or width>2048 or height>2048 or offset<table_end or offset>=len(raw):
            invalid.append({"frame":i,"width":width,"height":height,"offset":offset})
        records.append((width,height,offset))
    if invalid:
        return {"status":"unsupported-format","reason":"invalid width/height or frame offsets",
                "candidate_le_count":count,"invalid_examples":invalid[:3],
                "first_record":records[0]}
    frame_rows=[]
    for index,(width,height,off) in enumerate(records):
        next_offsets=[o for w,h,o in records if o>off]
        limit=min(next_offsets) if next_offsets else len(raw)
        if limit<=off:
            raise ValueError("invalid frame boundary")
        pos=off
        n_runs=0
        n_pixels=0
        minx,miny,maxx,maxy=width,height,-1,-1
        error=None
        sentinel=False
        while pos < limit:
            x=raw[pos]
            pos+=1
            if x==255:
                sentinel=True
                break
            if pos+2>limit:
                error="truncated run header";break
            y,length=raw[pos],raw[pos+1]
            pos+=2
            if length==0 or pos+length>limit or x+length>width or y>=height:
                error="invalid run coordinate/length"
                break
            n_runs+=1
            n_pixels+=length
            minx,miny=min(minx,x),min(miny,y)
            maxx,maxy=max(maxx,x+length-1),max(maxy,y)
            pos+=length
            if n_runs>100000:
                error="too many scanline runs";break
        entry={"frame":index,"width_px":width,"height_px":height,"data_offset":off,
               "next_boundary":limit,"decode_valid":sentinel and error is None,
               "run_count":n_runs,"painted_pixels":n_pixels}
        if maxx>=0:
            entry["painted_bbox"]={"x":minx,"y":miny,"width_px":maxx-minx+1,"height_px":maxy-miny+1}
        if error: entry["decode_error"]=error
        frame_rows.append(entry)
    sizes=collections.Counter((r["width_px"],r["height_px"]) for r in frame_rows)
    return {
        "status":"validated-frame-table" if all(r["decode_valid"] for r in frame_rows) else "validated-frame-table-but-run-format-unverified",
        "frames":count,
        "unique_frame_sizes":[{"width_px":w,"height_px":h,"frames":n}
                               for (w,h),n in sizes.most_common()],
        "max_size_px":{"width":max(r["width_px"] for r in frame_rows),
                       "height":max(r["height_px"] for r in frame_rows)},
        "scanlines_valid":sum(r["decode_valid"] for r in frame_rows),
        "frame_details":frame_rows,
    }

def parse_lbm(raw: bytes) -> dict:
    if len(raw)<12 or raw[:4]!=b"FORM":
        return {"status":"not-iff-form","prefix_hex":raw[:12].hex()}
    flavor=raw[8:12]
    if flavor not in (b"ILBM",b"PBM "):
        return {"status":"unrecognized-iff-subtype","subtype":flavor.decode("ascii","replace")}
    declared=struct.unpack_from(">I",raw,4)[0]
    pos=12
    chunks=[]
    width=height=planes=mask=compression=None
    while pos+8<=len(raw):
        label=raw[pos:pos+4]
        sz=struct.unpack_from(">I",raw,pos+4)[0]
        start=pos+8
        end=start+sz
        if end>len(raw):return {"status":"corrupt-iff-chunks"}
        chunks.append(label.decode("ascii","replace"))
        if label==b"BMHD" and sz>=20:
            width,height=struct.unpack_from(">HH",raw,start)
            planes,mask,compression=raw[start+8:start+11]
        pos=end+(sz&1)
    if width is None:return {"status":"iff-no-bmhd","subtype":flavor.decode("ascii")}
    return {"status":"validated-iff-bmhd",
            "iff_form":flavor.decode("ascii"),
            "width_px":width,"height_px":height,
            "planes":planes,"mask":mask,"compression":compression,
            "chunk_labels":chunks, "declared_form_bytes":declared}

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--epf",type=Path,required=True)
    parser.add_argument("--epf-root",type=Path,required=True)
    parser.add_argument("--output",type=Path,required=True)
    args=parser.parse_args()
    if sha256(args.epf.read_bytes())!=EXPECTED_EPF_SHA:
        raise SystemExit("REJECT original GOLF.EPF digest differs from selected v1.014")
    results=[]
    for ext,parserfn in ((".MCH",parse_mch),(".LBM",parse_lbm)):
        for file in sorted(args.epf_root.glob("*"+ext)):
            raw=file.read_bytes()
            outcome=parserfn(raw)
            results.append({"file":file.name,"bytes":len(raw),"sha256":sha256(raw),"format":ext[1:],**outcome})
    statuses=collections.Counter(r["status"] for r in results)
    out={
        "schema":"pixtee.reference.original_sprite_dimension_inventory.v1",
        "source":"Sensible Golf Windows PC v1.014 GOLF.EPF SHA-256 "+EXPECTED_EPF_SHA,
        "units":"original asset pixels, NOT world metres or final device pixels",
        "description":"Raw resource frame rectangles and optional painted bounds; references only; no pixel or palette content",
        "resource_count":len(results),
        "statuses":dict(statuses),
        "resources":results,
        "limitations":[
            "An original asset's frame rectangle may contain animation padding; painted_bbox is the drawn portion only.",
            "A frame table proves image dimensions, not which frames are actually used for golfers, flags, trees or impacts.",
            "Original rendering may crop, scale or arrange these frames; original on-screen footprint requires original renderer/load-call verification.",
            "Pixtee should upscale SOURCE artwork 2x/3x/4x for detail but draw to the SAME measured in-world bounds: source density does not change apparent actor size.",
            "Protected original pixels are not copied or output; final assets must be original and reviewed."
        ]
    }
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(out,indent=2)+"\n")
    print("SPRITE INVENTORY:",len(results),"resources;",dict(statuses))
    for r in results:
        print(r["file"],"status",r["status"],"frames",r.get("frames"),
              "size",r.get("width_px"),r.get("height_px"),
              "unique",r.get("unique_frame_sizes",[])[:5],
              "runs",r.get("scanlines_valid"))
    if not any(r["status"].startswith("validated-frame-table") for r in results):
        raise SystemExit("NO validated MCH frames: format research required")

if __name__=="__main__":
    main()
