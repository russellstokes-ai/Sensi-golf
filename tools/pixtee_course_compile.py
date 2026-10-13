#!/usr/bin/env python3
"""Compile frozen visual polygons into original MAPM/MAPI. Never alters inputs.

MAPS is a local 128x104-world-unit detail window, not a resized whole green.
Its 256x208 native coordinates map back to the unchanged course at exactly 2:1.
Live activation still requires authored slope data and native transition proof.
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


def green_detail_mapping(layout, origin):
    """Original MAPM marker -> MAPS sample coordinates; no sprite/physics scale.

    Source Windows 0x40AB1B subtracts the marker then doubles ball coordinates.
    Source flag coordinates subtract the marker without doubling. MAPS 0x409535
    is bounded to 256x208 native units; 2x2 native subcells cover 1x1 world units.
    Green scene admission is a distinct inclusive 104x76 region inside that map.
    The painted green may continue outside this window in the ordinary MAPM.
    """
    if tuple(layout['world']) != (544, 912):
        raise ValueError('Expected unchanged approved 544x912 world')
    if len(origin) != 2 or any(type(v) is not int for v in origin):
        raise ValueError('Native marker origin must be two integer coordinates')
    ox, oy = origin
    if ox % 16 or oy % 8 or ox < 0 or oy < 0 or ox+128 > 544 or oy+104 > 912:
        raise ValueError('Detail origin must be an in-world original MAPM tile')
    flag = next(p for p in layout['placements'] if p['type'] == 'flag')
    if not ox+8 <= flag['x'] <= ox+112 or not oy+8 <= flag['y'] <= oy+84:
        raise ValueError('Cup must lie in the source green scene admission region')
    green = layout['surfaces']['green']
    bounds = [min(p[0] for p in green), min(p[1] for p in green),
              max(p[0] for p in green), max(p[1] for p in green)]
    samples = [[ox+x+0.5, oy+y+0.5] for y in range(104) for x in range(128)]
    return {'schema':'pixtee.native-green-detail-mapping.v1',
            'world_origin':[ox,oy], 'native_tiles':[16,26],
            'native_size':[256,208], 'native_subcells':[128,104],
            'world_window':[128,104], 'course_scale':1,
            'cup_local_native':[2*(flag['x']-ox),2*(flag['y']-oy)],
            'cup_source_flag':[flag['x']-ox,flag['y']-oy],
            'source_admission_world':[ox+8,oy+8,ox+112,oy+84],
            'green_world_bounds':bounds,
            'entire_green_in_window':all(ox <= x < ox+128 and oy <= y < oy+104
                                        for x,y in green),
            'sample_world':samples,
            'sample_green_mask':[inside(green,x,y) for x,y in samples],
            'live_physics_enabled':False,
            'remaining':['authored_slope_field','incoming_shot_green_support',
                         'source_scene_activation_and_cup_integration']}

def compile_maps_window(layout, origin, slope_cells, descriptor_bank=b'', selector_bank=b''):
    """Compile explicitly authored sectors/magnitudes into real original MAPS.

    Existing MAPI entries retain their indices and bytes. Missing slope cells
    fail rather than silently inventing a slope. This produces a resource for
    native validation, not authorization to bypass incomplete gameplay gates.
    """
    mapping = green_detail_mapping(layout, origin)
    if len(descriptor_bank) != len(selector_bank) or len(descriptor_bank) % 8:
        raise ValueError('Shared native MAPI banks must have matching aligned lengths')
    descriptors, selectors = bytearray(descriptor_bank), bytearray(selector_bank)
    tiles = {descriptor_bank[i:i+8]+selector_bank[i:i+8]:i//8
             for i in range(0,len(descriptor_bank),8)}
    samples = []
    cup = next(p for p in layout['placements'] if p['type'] == 'flag')
    for (wx,wy),on_green in zip(mapping['sample_world'],mapping['sample_green_mask']):
        at = (int(wx),int(wy))
        surfaces = layout['surfaces']
        # Approved course.js gives hazards priority even when polygons overlap.
        # Detail lookup must preserve that order, including inside the green.
        hazards = [(6,surfaces['water']),(6,surfaces['river'])]
        hazards += [(5,p) for p in surfaces['bunkers']]
        hazard = next((d for d,p in hazards if p and inside(p,wx,wy)),None)
        if hazard is not None:
            samples.append(hazard)
            continue
        if not on_green:
            # Preserve original visual surface priority at this finer resolution.
            polygons = [(1,surfaces['tee']),(1,surfaces['fairway'])]
            samples.append(next((d for d,p in polygons if p and inside(p,wx,wy)),3))
            continue
        if at not in slope_cells:
            raise ValueError('Authored native slope missing at green world cell '+str(at))
        sector,magnitude = slope_cells[at]
        if any(type(v) is not int or not 0 <= v <= 15 for v in (sector,magnitude)):
            raise ValueError('Original green slope requires sector and magnitude in 0..15')
        word = 31 | sector<<8 | magnitude<<12
        # Original GREEN H1 is the genuine code8 cup cell, not a proximity FX.
        samples.append(7 if at == (int(cup['x']),int(cup['y'])) else word)
    maps = bytearray(0x60)
    struct.pack_into('>HH',maps,0x54,16,26)
    for ty in range(26):
        for tx in range(16):
            cells = [samples[(ty*4+sy)*128+tx*8+sx] for sy in range(4) for sx in range(8)]
            words,masks = encode_tile(cells)
            key = words+masks
            if key not in tiles:
                if len(descriptors)//8 >= 1024: raise ValueError('Original ten-bit MAPI bank exhausted')
                tiles[key] = len(descriptors)//8
                descriptors.extend(words)
                selectors.extend(masks)
            maps.extend(struct.pack('>H',tiles[key]))
    return {'MAPS.bin':bytes(maps),'MAPI-descriptors.bin':bytes(descriptors),
            'MAPI-selectors.bin':bytes(selectors)}


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
