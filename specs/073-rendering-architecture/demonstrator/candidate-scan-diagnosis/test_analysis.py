#!/usr/bin/env python3
"""Synthetic regression checks; no benchmark processes."""
import copy
import json
from pathlib import Path
import tempfile
import unittest

import analyze


class AnalysisTests(unittest.TestCase):
    def test_pair_normalization_and_null(self):
        a = dict(elapsedNs=40, applicableParagraphs=4)
        b = dict(elapsedNs=60, applicableParagraphs=4)
        self.assertEqual(analyze.pair_value(a, b), (20, 5))
        null = dict(elapsedNs=None, applicableParagraphs=0)
        self.assertEqual(analyze.pair_value(null, null), (None, None))
        for bad in (dict(elapsedNs=60, applicableParagraphs=3), null):
            with self.assertRaises(ValueError):
                analyze.pair_value(a, bad)

    def test_constant_extreme_signs(self):
        self.assertEqual(analyze.describe([3] * 20)['mad'], 0)
        result = analyze.describe([-1, 0, 1, 10**12])
        self.assertEqual((result['positive'], result['negative'], result['zero']), (2, 1, 1))
        self.assertEqual(result['maximum'], 10**12)
        self.assertEqual(result['median'], .5)

    def test_order_and_halves(self):
        cell = dict(case='fixture', variant='zero', operation='reflow', applicable=True, gate='pass')
        pairs = [dict(sample=i, armOrder=['baseline', 'retained'] if i % 2 == 0 else ['retained', 'baseline'],
                      differenceNs=i * 2, overheadPerParagraphNs=i,
                      baseline=dict(elapsedNs=i), retained=dict(elapsedNs=i * 3)) for i in range(20)]
        result = analyze.cell_analysis(cell, pairs)['descriptive']
        self.assertEqual(result['orderMedians'], dict(baseline=9, retained=10))
        self.assertEqual(result['halves']['firstTen']['overheadPerParagraphNs'], 4.5)
        self.assertEqual(result['halves']['lastTen']['baselineMedianNs'], 14.5)
        self.assertEqual(result['halves']['lastTen']['retainedMedianNs'], 43.5)
        for bad in (pairs[:-1], pairs[:-1] + [pairs[0]]):
            with self.assertRaises(ValueError):
                analyze.cell_analysis(cell, bad)

    def test_timer_suppression(self):
        report = analyze.load_report()
        a = dict(elapsedNs=10, applicableParagraphs=1, clockPeriodNs=1, minimumPositiveDeltaNs=12)
        result = report.summarize([(a, dict(a, elapsedNs=20))] * 20, [list(range(20))] * 2)
        self.assertTrue(result['timerLimited'])
        self.assertIsNone(result['medianImprovement'])
        self.assertIsNone(result['intervals95']['medianImprovement'])
        self.assertEqual(result['overheadPerParagraphNs'], 10)

    def test_summary_disagreement(self):
        expected = dict(cells=[dict(gate='regression-fail')] * 8)
        analyze.compare_summary(copy.deepcopy(expected), expected)
        with self.assertRaisesRegex(ValueError, 'disagreement'):
            analyze.compare_summary({}, expected)

    def test_identity_and_raw_rejection(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            raw = root / 'raw.jsonl'
            raw.write_text('{}\n')
            fixture = root / 'fixture.json'
            fixture.write_text('{}\n')
            entries = [dict(id=str(i), pair='pair', order=i, label=arm, sample=0,
                            case='fixture') for i, arm in enumerate(('baseline', 'retained'))]
            records = [dict(**e, status='completed', raw=raw.name, rawSha256=analyze.digest(raw),
                            fixtureSha256=analyze.digest(fixture)) for e in entries]
            analyze.verify_records(root, entries, records)
            for bad in (records[:-1], [records[0], records[0]], list(reversed(records))):
                with self.assertRaises(ValueError):
                    analyze.verify_records(root, entries, bad)
            analyze.verify_identities({str(raw): analyze.digest(raw)})
            with self.assertRaises(ValueError):
                analyze.verify_identities({str(raw): 'bad'})
            with self.assertRaises(ValueError):
                analyze.verify_identities({str(root / 'missing'): 'bad'})
            raw.write_text('altered')
            with self.assertRaisesRegex(ValueError, 'hash mismatch'):
                analyze.verify_records(root, entries, records)

    def test_null_cell_and_failed_excess(self):
        cell = dict(case='f', variant='zero', operation='metadata', applicable=False)
        pairs = [dict(sample=i, differenceNs=None) for i in range(20)]
        self.assertIsNone(analyze.cell_analysis(cell, pairs)['descriptive'])
        pairs[0]['differenceNs'] = 0
        with self.assertRaises(ValueError):
            analyze.cell_analysis(cell, pairs)

    def test_archive_rejection(self):
        import hashlib
        import io
        import tarfile
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            archive = root / 'evidence.tar.gz'
            with tarfile.open(archive, 'w:gz') as tar:
                member = tarfile.TarInfo('sample')
                member.size = 3
                tar.addfile(member, io.BytesIO(b'raw'))
            manifest = dict(archives={archive.name: dict(sha256=analyze.digest(archive),
                bytes=archive.stat().st_size, files={'sample': hashlib.sha256(b'raw').hexdigest()})})
            analyze.write(root / 'archives.json', manifest)
            analyze.verify_archives(root, root / 'good')
            manifest['archives'][archive.name]['files']['sample'] = 'bad'
            analyze.write(root / 'archives.json', manifest)
            with self.assertRaisesRegex(ValueError, 'member hash'):
                analyze.verify_archives(root, root / 'bad-member')
            manifest['archives'][archive.name]['sha256'] = 'bad'
            analyze.write(root / 'archives.json', manifest)
            with self.assertRaisesRegex(ValueError, 'hash mismatch'):
                analyze.verify_archives(root, root / 'bad-archive')

    def test_alternation(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'x').write_text('{}')
            (root / 'f.json').write_text('{}')
            for sample in range(20):
                arms = ('retained', 'baseline') if sample % 2 else ('baseline', 'retained')
                entries = [dict(id=str(i), pair='p', order=i, sample=sample, label=a, case='f')
                           for i, a in enumerate(arms)]
                rows = [dict(**e, status='completed', raw='x', rawSha256=analyze.digest(root / 'x'),
                             fixtureSha256=analyze.digest(root / 'f.json')) for e in entries]
                analyze.verify_records(root, entries, rows)


if __name__ == '__main__':
    unittest.main()
