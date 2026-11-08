"""Refinement-specific offline regression tests; all launches forbidden."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import run as p
import report as q
import test_pilot as base

class Refinement(unittest.TestCase):
    def setUp(self):
        self.guard=p.a.no_launches();self.guard.__enter__()
    def tearDown(self):self.guard.__exit__(None,None,None)
    def test_mixed_suppression(self):
        records,raw=base.synthetic()
        for e in records:
            if e['mode']=='timing' and e['round']==0:
                for row in raw[e['id']]:row['minimumPositiveDeltaNs']=10**9
        result=q.analyze(records,raw,True)
        summary=result['cells']['batch-96-enabled']['operations']['prepared-reflow']['groups']['relativeDifference']
        self.assertEqual((summary['available'],summary['suppressed']),(7,1))
        self.assertEqual(result['telemetryComparisons'][0]['summary']['relativeDifference']['suppressed'],1)
        self.assertTrue(all(g['relativeDifferenceReason'] for g in result['groups'] if g['relativeDifference'] is None))
        self.assertNotIn('batchTiming',result['cells']['original-reference'])
    def test_forensic_partial_and_no_resume(self):
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp)/'out';out.mkdir()
            (out/'attempts.jsonl').write_text('{"id":"attempted"}\n')
            (out/'index.jsonl').write_text('{broken')
            (out/'partial.jsonl').write_text('partial bytes')
            result=q.report(out)
            self.assertFalse(result['complete']);self.assertIsNone(result['selection'])
            self.assertTrue(result['validationErrors']);self.assertIn('partial.jsonl',result['preservedFiles'])
            with self.assertRaises(Exception):p.resume(out,Path(tmp)/'resume')
    def test_completed_prefix_recovery(self):
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp)/'out';out.mkdir();p.write(out/'environment.json',p.environment())
            with patch.object(p,'validate',return_value=[{'completed':True}]):
                self.assertEqual(p.resume(out,Path(tmp)/'next'),[{'completed':True}])
            with patch.object(p,'validate',return_value=[{}]),patch.object(p,'environment',return_value={}):
                with self.assertRaises((ValueError,AssertionError)):p.resume(out,Path(tmp)/'mismatch')

if __name__=='__main__':unittest.main()
