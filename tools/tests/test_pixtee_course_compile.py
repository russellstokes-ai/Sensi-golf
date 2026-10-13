import importlib.util
from pathlib import Path
import unittest

MODULE = Path(__file__).parents[1] / 'pixtee_course_compile.py'

class CourseCompileTests(unittest.TestCase):
    def test_authored_green_bundle_preserves_world_map_and_bank_prefix(self):
        import struct
        spec = importlib.util.spec_from_file_location('compiler', MODULE)
        compiler = importlib.util.module_from_spec(spec);spec.loader.exec_module(compiler)
        layout = {'world':[544,912], 'surfaces':{'green':[[310,112],[408,112],[408,222],[310,222]],'water':[],'river':[],'bunkers':[],'tee':[],'fairway':[]},'placements':[{'type':'flag','x':364,'y':168}]}
        mapm=bytearray(0x60+34*114*2);struct.pack_into('>HH',mapm,0x54,34,114)
        desc,selectors=compiler.encode_tile([3]*32)
        source={'MAPM.bin':bytes(mapm),'SPT.bin':bytes(50),'MAPI-descriptors.bin':desc,'MAPI-selectors.bin':selectors}
        before=dict(source)
        authored={'schema':'pixtee.authored-green-slopes.v1','origin':[304,120],'sector':0,'magnitude':1}
        result=compiler.compile_authored_green_resources(layout,authored,source)
        self.assertEqual(source,before)
        self.assertEqual(result['SPT.bin'],source['SPT.bin'])
        self.assertTrue(result['MAPI-descriptors.bin'].startswith(desc))
        self.assertTrue(result['MAPI-selectors.bin'].startswith(selectors))
        changed=[]
        for at in range(0x60,len(mapm),2):
            old=struct.unpack_from('>H',mapm,at)[0];new=struct.unpack_from('>H',result['MAPM.bin'],at)[0]
            self.assertEqual(old&1023,new&1023)
            if old!=new: changed.append((at-0x60)//2)
        self.assertEqual(changed,[15*34+19,15*34+20,15*34+21])
        self.assertEqual(len(result['expected-detail-words.bin']),128*104*2)
        for invalid in [{'sector':16},{'magnitude':0},{'origin':[305,120]}]:
            with self.assertRaises(ValueError): compiler.compile_authored_green_resources(layout,{**authored,**invalid},source)

    def test_detail_window_preserves_hazard_priority_over_green(self):
        spec = importlib.util.spec_from_file_location('compiler', MODULE)
        compiler = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(compiler)
        layout = {'world':[544,912], 'surfaces':{
            'green':[[310,112],[408,112],[408,222],[310,222]],
            'water':[[310,120],[320,120],[320,128],[310,128]],
            'river':[], 'bunkers':[], 'tee':[], 'fairway':[]},
            'placements':[{'type':'flag','x':364,'y':168}]}
        mapping = compiler.green_detail_mapping(layout,(304,120))
        field = {(int(x),int(y)):(3,2) for x,y in mapping['sample_world']
                 if compiler.inside(layout['surfaces']['green'],x,y)
                 and not compiler.inside(layout['surfaces']['water'],x,y)}
        detail = compiler.compile_maps_window(layout,(304,120),field)
        import struct
        # World (312.5,120.5) is visibly water despite green polygon overlap.
        tile = struct.unpack_from('>H',detail['MAPS.bin'],0x60+2)[0]
        descriptor = struct.unpack_from('>H',detail['MAPI-descriptors.bin'],tile*8)[0]
        self.assertEqual(descriptor,6)

    def test_green_detail_uses_native_window_without_resizing_green(self):
        spec = importlib.util.spec_from_file_location('compiler', MODULE)
        compiler = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(compiler)
        layout = {'world':[544,912], 'surfaces':{'green':[[310,112],[408,112],[408,222],[310,222]],'water':[],'river':[],'bunkers':[],'tee':[],'fairway':[]},'placements':[{'type':'flag','x':364,'y':168}]}
        self.assertTrue(hasattr(compiler,'green_detail_mapping'), 'Native detail-coordinate compiler is missing')
        mapping = compiler.green_detail_mapping(layout,(304,120))
        self.assertEqual(mapping['native_tiles'],[16,26])
        self.assertEqual(mapping['native_size'],[256,208])
        self.assertEqual(mapping['world_origin'],[304,120])
        self.assertEqual(mapping['cup_local_native'],[120,96])
        self.assertEqual(mapping['cup_source_flag'],[60,48])
        self.assertEqual(mapping['course_scale'],1)
        self.assertEqual(mapping['green_world_bounds'],[310,112,408,222])
        # Full green containment is not a prerequisite for a detail window.
        self.assertFalse(mapping['entire_green_in_window'])
        self.assertEqual(mapping['sample_world'][0],[304.5,120.5])
        self.assertEqual(mapping['sample_world'][-1],[431.5,223.5])
        with self.assertRaises(ValueError): compiler.green_detail_mapping(layout,(305,120))
        self.assertTrue(hasattr(compiler,'compile_maps_window'))
        field = {(int(x),int(y)):(3,2) for x,y in mapping['sample_world'] if compiler.inside(layout['surfaces']['green'],x,y)}
        detail = compiler.compile_maps_window(layout,(304,120),field)
        self.assertEqual(len(detail['MAPS.bin']),0x60+16*26*2)
        # Inspect every original MAPS 2x2-native/1x1-world subcell.
        import struct
        for index,(wx,wy) in enumerate(mapping['sample_world']):
            x,y = index%128,index//128
            tile = struct.unpack_from('>H',detail['MAPS.bin'],0x60+2*((y//4)*16+x//8))[0]
            mask = 0x80 >> (x%8)
            off = tile*8
            selector = bool(detail['MAPI-selectors.bin'][off+y%4]&mask)+2*bool(detail['MAPI-selectors.bin'][off+y%4+4]&mask)
            word = struct.unpack_from('>H',detail['MAPI-descriptors.bin'],off+selector*2)[0]
            if (int(wx),int(wy)) == (364,168): self.assertEqual(word,7)
            elif compiler.inside(layout['surfaces']['green'],wx,wy): self.assertEqual(word,31 | (3<<8) | (2<<12))
            else: self.assertEqual(word,3)
        with self.assertRaises(ValueError): compiler.compile_maps_window(layout,(304,120),{})

    def test_original_mapi_preserves_all_32_subcells(self):
        self.assertTrue(MODULE.exists(), 'Approved polygon compiler is missing')
        spec = importlib.util.spec_from_file_location('compiler', MODULE)
        compiler = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(compiler)
        samples = [1, 3, 5, 6] * 8
        descriptors, selectors = compiler.encode_tile(samples)
        self.assertEqual(len(descriptors), 8)
        self.assertEqual(len(selectors), 8)
        for i, expected in enumerate(samples):
            row, col = divmod(i, 8)
            mask = 0x80 >> col
            index = bool(selectors[row] & mask) + 2 * bool(selectors[row + 4] & mask)
            self.assertEqual(int.from_bytes(descriptors[index*2:index*2+2], 'big'), expected)
        with self.assertRaises(ValueError):
            compiler.encode_tile([1, 2, 3, 4, 5] + [1] * 27)

if __name__ == '__main__':
    unittest.main()
