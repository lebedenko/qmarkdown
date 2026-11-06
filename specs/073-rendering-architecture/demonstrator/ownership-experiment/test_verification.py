"""Focused rejection tests for differential checks and archive identities."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
from verify import compare, validate_manifest, digest


class VerificationTests(unittest.TestCase):
    def pair(self):
        bucket = dict(calls=2, runVisits=3, reorderCalls=2, slices=2, ownerComparisons=0)
        work = dict(candidates=2, whitespaceVisits=1, fitting=bucket.copy(), formatting=bucket.copy(),
                    ownerBuilds=0, ownerExaminations=0, ownerLookups=4, ownerCapacity=1, ownerPayloadBytes=8)
        rows = [dict(phase=p, geometry=[dict(runs=[{}])], shapeDeltas=[0], counters=[copy.deepcopy(work)])
                for p in ('initial', 'reflow', 'repeat', 'metadata')]
        return rows, copy.deepcopy(rows)

    def test_equivalence(self):
        compare(*self.pair(), 'instrumented')

    def test_geometry_and_each_preserved_counter(self):
        for field in ('calls', 'runVisits', 'reorderCalls', 'slices'):
            a,b=self.pair();a[0]['counters'][0]['fitting'][field]+=1
            with self.assertRaises(AssertionError):compare(a,b,'instrumented')
        a,b=self.pair();a[0]['geometry']=[{}]
        with self.assertRaises(AssertionError):compare(a,b,'headline')

    def test_fallback_and_reflow_build_rejected(self):
        for field in ('ownerBuilds', 'ownerExaminations'):
            a,b=self.pair();a[1]['counters'][0][field]=1
            with self.assertRaises(AssertionError):compare(a,b,'instrumented')
        a,b=self.pair();a[0]['counters'][0]['fitting']['ownerComparisons']=1
        with self.assertRaises(AssertionError):compare(a,b,'instrumented')

    def test_baseline_corruption_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);(root/'archives.json').write_text('{}')
            (root/'part.tar.gz').write_bytes(b'archive');(root/'trace').write_bytes(b'original')
            manifest={'archives':{'part.tar.gz':{'sha256':digest(root/'part.tar.gz'),
                                              'files':{'trace':digest(root/'trace')}}}}
            validate_manifest(root,root,manifest)
            (root/'trace').write_bytes(b'changed')
            with self.assertRaises(AssertionError):validate_manifest(root,root,manifest)


if __name__=='__main__':unittest.main()
