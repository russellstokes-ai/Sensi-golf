#!/usr/bin/env python3
"""Compile frozen visual polygons into original MAPM/MAPI. Never alters inputs.

MAPS coverage remains fail-closed: the source-authentic detail window is 128x104
world units and cannot contain this approved green's full vertical extent.
No coordinate scaling or invented slopes are permitted to hide that mismatch.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import zipfile

LAKEWOOD_SHA = '82c0df6831241e31ad0c83a4302df145349afb9b63503487a914b964d05c7ff3'

def encode_tile(samples):
    if len(samples) != 32:
        raise ValueError('Original tile requires 32 subcells')
    words = list(dict.fromkeys(samples))
    if len(words) > 4:
        raise ValueError('Original MAPI cannot encode more than four distinct descriptors')
    selectors = bytearray(8)
    for i, value in enumerate(samples):
        row, col = divmod(i, 8)
        index = words.index(value)
        if index & 1:
            selectors[row] |= 0x80 >> col
        if index & 2:
            selectors[row + 4] |= 0x80 >> col
    return struct.pack('>4H', *(words + [0] * (4-len(words)))), bytes(selectors)

def inside(polygon, x, y):
    result = False
    previous = polygon[-1]
    for point in polygon:
        ax, ay = point
        bx, by = previous
        if (ay > y) != (by > y) and x < (bx-ax)*(y-ay)/(by-ay)+ax:
            result = not result
        previous = point
    return result

def raster(layout):
    """Sample the actual 2x2 lookup cells, at their centre; no map scaling."""
    width, height = layout['world']
    if (width, height) != (544, 912):
        raise ValueError('Frozen Lakewood must be exactly 544x912')
    surfaces = layout['surfaces']
    # Priorities are exactly course.js surface(): water, sand, green, fairway,
    # path, rough. Paths have no recovered authoritative lie assignment yet;
    # retain rough and report that gate instead of inventing a terrain profile.
    polygons = [(6, surfaces['water']), (6, surfaces['river'])]
    polygons += [(5, p) for p in surfaces['bunkers']]
    polygons += [(31, surfaces['green']), (1, surfaces['tee']), (1, surfaces['fairway'])]
    rows = []
    for y in range(1, height, 2):
        # Scanline intersections avoid evaluating hundreds of polygon edges for
        # every cell. The parity rule is the same as the approved HTML renderer.
        spans = []
        for descriptor, polygon in polygons:
            crossings = []
            previous = polygon[-1]
            for ax, ay in polygon:
                bx, by = previous
                if (ay > y) != (by > y):
                    crossings.append((bx-ax)*(y-ay)/(by-ay)+ax)
                previous = (ax, ay)
            crossings.sort()
            spans.append((descriptor, list(zip(crossings[::2], crossings[1::2]))))
        row = bytearray()
        for x in range(1, width, 2):
            row.append(next((d for d, intervals in spans
                             if any(a <= x < b for a, b in intervals)), 3))
        rows.append(row)
    return rows

def compile_course(source, output):
    source, output = Path(source).resolve(), Path(output).resolve()
    digest = hashlib.sha256(source.read_bytes()).hexdigest()
    if digest != LAKEWOOD_SHA:
        raise ValueError('Source ZIP does not match owner-approved SHA-256')
    if output.exists():
        raise ValueError('Refusing to overwrite a compiled checkpoint')
    with zipfile.ZipFile(source) as archive:
        if archive.testzip() is not None:
            raise ValueError('Frozen ZIP CRC failure')
        matches = [n for n in archive.namelist() if n.endswith('course-layout.json')]
        if len(matches) != 1:
            raise ValueError('Expected one frozen layout')
        layout_bytes = archive.read(matches[0])
        layout = json.loads(layout_bytes)
    rows = raster(layout)
    descriptors, selectors, tile_ids = bytearray(), bytearray(), {}
    mapm = bytearray(0x60)
    struct.pack_into('>HH', mapm, 0x54, 34, 114)
    for ty in range(114):
        for tx in range(34):
            samples = [rows[ty*4+sy][tx*8+sx] for sy in range(4) for sx in range(8)]
            words, masks = encode_tile(samples)
            key = words + masks
            if key not in tile_ids:
                if len(tile_ids) >= 1024:
                    raise ValueError('Original ten-bit MAPI tile bank exhausted')
                tile_ids[key] = len(tile_ids)
                descriptors.extend(words)
                selectors.extend(masks)
            mapm.extend(struct.pack('>H', tile_ids[key]))
    # Cup and golfer positions come from approved placements, not fixture data.
    golfer = next(p for p in layout['placements'] if p['type'] == 'golfer')
    flag = next(p for p in layout['placements'] if p['type'] == 'flag')
    spt = bytearray(50)
    for player in range(4):
        struct.pack_into('>HH', spt, player*10+4, int(golfer['x']), int(golfer['y']))
    struct.pack_into('>HH', spt, 44, int(flag['x']), int(flag['y']))
    green = layout['surfaces']['green']
    bounds = [min(p[0] for p in green), min(p[1] for p in green),
              max(p[0] for p in green), max(p[1] for p in green)]
    payloads = {'MAPM.bin': bytes(mapm), 'SPT.bin': bytes(spt),
                'MAPI-descriptors.bin': bytes(descriptors),
                'MAPI-selectors.bin': bytes(selectors),
                'expected-subcells.bin': b''.join(rows), 'course-layout.json': layout_bytes}
    manifest = {
        'schema': 'pixtee.approved-polygon-compile.v1', 'source_zip_sha256': digest,
        'world': [544,912], 'mapm_tiles': [34,114], 'cell_size': [2,2],
        'coordinate_scale': 1, 'tile_count': len(tile_ids),
        'tee': [int(golfer['x']), int(golfer['y'])],
        'cup': [int(flag['x']), int(flag['y'])], 'green_bounds': bounds,
        'maps_native_world_window': [128,104],
        'maps_enabled': False, 'authored_slope_field_present': False,
        'open_gates': ['approved_green_exceeds_native_MAPS_height',
                       'authored_slope_data_not_supplied', 'path_lie_not_source_verified',
                       'tree_object_collision_not_integrated', 'playable_scene_not_verified'],
        'sampling': 'polygon centre at each original 2x2 cell; maximum one unit per axis',
        'files': {n: {'sha256': hashlib.sha256(b).hexdigest(), 'size': len(b)}
                  for n,b in payloads.items()}}
    output.mkdir(parents=True)
    for name, data in payloads.items():
        (output/name).write_bytes(data)
    (output/'manifest.json').write_text(json.dumps(manifest, indent=2)+'\n', encoding='utf-8')
    return manifest

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-zip', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    print(json.dumps(compile_course(args.source_zip, args.output), indent=2))
