import sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from compare_full_oracle import compare

def trace():
    sample={"tick":0,"x":1,"y":2,"height":3,"vertical_force":4,"horizontal_force":5,"direction":6,"swing_adjuster":0,"adjusted_power":100}
    return {"surface_code":0,"samples":[sample],"events":{"landing":{"tick":1,"x":10,"y":20},"rest":{"tick":2,"x":30,"y":40}}}

class Tests(unittest.TestCase):
    def test_exact(self): self.assertTrue(compare(trace(),trace())["pass"])
    def test_event(self):
        a=trace(); b=trace(); b["events"]["rest"]["x"]=31
        self.assertFalse(compare(a,b)["pass"])
    def test_state(self):
        a=trace(); b=trace(); b["samples"][0]["vertical_force"]=99
        self.assertFalse(compare(a,b)["pass"])
if __name__=="__main__":unittest.main()
