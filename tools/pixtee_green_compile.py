#!/usr/bin/env python3
"""Create a separate hash-verified authored MAPS bundle; never overwrite inputs."""
import argparse,hashlib,json
from pathlib import Path
from pixtee_course_compile import compile_authored_green_resources,LAKEWOOD_SHA

def compile_green(course,authored_file,output):
    course,authored_file,output=Path(course).resolve(),Path(authored_file).resolve(),Path(output).resolve()
    if output.exists(): raise ValueError('Refusing to overwrite a green checkpoint')
    manifest=json.loads((course/'manifest.json').read_text())
    if manifest['source_zip_sha256']!=LAKEWOOD_SHA:raise ValueError('Approved Lakewood source required')
    source={}
    for name,record in manifest['files'].items():
        data=(course/name).read_bytes()
        if hashlib.sha256(data).hexdigest()!=record['sha256']:raise ValueError('Source hash mismatch: '+name)
        source[name]=data
    authored=json.loads(authored_file.read_text())
    if hashlib.sha256(source['course-layout.json']).hexdigest()!=authored['layout_sha256']:
        raise ValueError('Authored field does not match frozen course layout')
    payload=compile_authored_green_resources(json.loads(source['course-layout.json']),authored,source)
    output.mkdir(parents=True)
    for name,data in payload.items(): (output/name).write_bytes(data)
    record={'schema':'pixtee.authored-green-compile.v1','source_zip_sha256':manifest['source_zip_sha256'],
            'authored_field_sha256':hashlib.sha256(authored_file.read_bytes()).hexdigest(),
            'origin':authored['origin'],'native_detail_size':[256,208],'world_detail_size':[128,104],
            'source_bank_prefix_preserved':True,'course_scale':1,
            'files':{n:{'sha256':hashlib.sha256(b).hexdigest(),'size':len(b)} for n,b in payload.items()}}
    (output/'manifest.json').write_text(json.dumps(record,indent=2)+'\n')
    return record

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--course',required=True);parser.add_argument('--authored',required=True);parser.add_argument('--output',required=True)
    args=parser.parse_args()
    print(json.dumps(compile_green(args.course,args.authored,args.output),indent=2))
