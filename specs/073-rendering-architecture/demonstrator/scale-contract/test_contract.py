import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import prepare
import diagnose

class Contract(unittest.TestCase):
    def test_generation_after_motion(self):
        with tempfile.TemporaryDirectory() as temp:
            path=Path(temp)/'source';prepare.generate(path)
            text=(path/'native.cpp').read_text()
            self.assertLess(text.index('offset+=step;'),text.index('mappingState.update(mapping)'))
            self.assertLess(text.index('mappingState.update(mapping)'),text.index('auto *canvas=surface->getCanvas();'))
            self.assertIn('auto p=queryMapping().windowToDocument({x,y});',text)
            self.assertIn('document.layout(layoutWidth)',text)
            self.assertNotIn('SDL_WINDOW_ALLOW_HIGHDPI',text)
    def test_anchor_drift(self):
        for text in ('','x x'):
            with self.assertRaises(ValueError):prepare.replace(text,'x','y')
    def test_duplicate_before_launch(self):
        with tempfile.TemporaryDirectory() as temp:
            path=Path(temp)
            (path/'attempts.jsonl').write_text('{"event":"start","id":"d001"}\n')
            with patch.object(diagnose.subprocess,'Popen',side_effect=AssertionError('launched')):
                with self.assertRaisesRegex(ValueError,'duplicate'):
                    diagnose.run(path,'d001',1.25,'hypothesis','expected',['missing'])
    def test_reserves_final_budget(self):
        with tempfile.TemporaryDirectory() as temp:
            path=Path(temp)
            import json
            (path/'attempts.jsonl').write_text(''.join(json.dumps(dict(event='start',id=str(i)))+'\n' for i in range(60)))
            with self.assertRaisesRegex(ValueError,'final 690 reserved'):
                diagnose.run(path,'new',1.25,'hypothesis','expected',['missing'])
    def test_wrong_output_and_scale(self):
        for output in (dict(enabled=True,name='DP-2',scale=1.25),dict(enabled=True,name='DP-1',scale=1)):
            with self.assertRaises((RuntimeError,ValueError)):
                diagnose.previous.output_scale([output],1.25)
    def test_limits(self):
        self.assertIsNotNone(diagnose.previous.limits(121,0,200,0))
        self.assertIsNotNone(diagnose.previous.limits(21,0,20,0))
        self.assertIsNotNone(diagnose.previous.limits(1,0,20,5*diagnose.previous.GIB))

if __name__=='__main__':unittest.main()

class CaptureGeneration(unittest.TestCase):
    def test_missing_stale_and_wrong_buffer(self):
        import run
        mapping=dict(stage='scale-contract',generation=2,buffer=[1000,750],window=[800,600],zoom=1)
        capture=dict(stage='capture-mapping',generation=2,buffer=[1000,750],window=[800,600],zoom=1)
        self.assertEqual(run.mapping_assertions([mapping,capture]),[])
        for changed in (dict(generation=1),dict(buffer=[800,600]),dict(zoom=2)):
            self.assertTrue(run.mapping_assertions([mapping,{**capture,**changed}]))
        self.assertTrue(run.mapping_assertions([capture]))
        self.assertTrue(run.mapping_assertions([mapping,mapping]))

class IndependentGates(unittest.TestCase):
    def test_stale_density_does_not_hide_physical_size(self):
        from report import gates
        mapping=dict(surface=[800,600],buffer=[1600,1200],forward=[2,2,48,48],zoom=1,logical=[800,600],density=2)
        self.assertEqual(gates(mapping,1.25),dict(physicalComparability=True,rasterDensity=False))
        mapping['forward']=[1,1,24,24]
        self.assertEqual(gates(mapping,2),dict(physicalComparability=False,rasterDensity=True))
