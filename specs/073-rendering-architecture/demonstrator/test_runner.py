import os
import json
from pathlib import Path
import sys
import tempfile
import unittest
from run import supervise


class SupervisorTests(unittest.TestCase):
    def test_fixture_spans(self):
        data=json.loads((Path(__file__).parent/'fixtures.json').read_text())
        for fixture in data['fixtures']:
            for block in fixture['blocks']:
                source=block['text'].encode('utf-16-le')
                for span in block['spans']+block['objects']:
                    self.assertGreater(span['length'],0)
                    self.assertGreaterEqual(span['start'],0)
                    self.assertLessEqual(2*(span['start']+span['length']),len(source))
                for obj in block['objects']:
                    self.assertEqual(source[2*obj['start']:2*(obj['start']+obj['length'])].decode('utf-16-le'),'[object]')

    def run_child(self, code, expected=None):
        with tempfile.TemporaryDirectory() as directory:
            return supervise([sys.executable, "-u", "-c", code], Path(directory)/"run",
                             os.environ.copy(), expected or {}, [], (.3,.3,.3))

    def test_success(self):
        self.assertTrue(self.run_child('print(\'{"stage":"exit"}\')')["passed"])

    def test_timeout(self):
        result=self.run_child('import time; print(\'{"stage":"ready"}\'); time.sleep(5)')
        self.assertTrue(result["timeout"])
        self.assertEqual(result["lastStage"], "ready")

    def test_crash(self):
        self.assertFalse(self.run_child("import os; os.abort()")["passed"])

    def test_malformed(self):
        self.assertFalse(self.run_child('print("bad");print(\'{"stage":"exit"}\')')["passed"])

    def test_truncated(self):
        self.assertFalse(self.run_child('print(\'{"stage":\',end="")')["passed"])

    def test_wrong_backend(self):
        self.assertFalse(self.run_child('print(\'{"stage":"ready","api":3}\');print(\'{"stage":"exit"}\')',{"api":5})["passed"])

    def test_wrong_dpr(self):
        self.assertFalse(self.run_child('print(\'{"stage":"ready","dpr":1.25}\');print(\'{"stage":"exit"}\')',{"dpr":1})["passed"])

    def test_failed_write(self):
        self.assertFalse(self.run_child('print(\'{"stage":"capture-failed"}\');raise SystemExit(4)')["passed"])


if __name__ == "__main__":
    unittest.main()
