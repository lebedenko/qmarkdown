import json
from pathlib import Path
import sys
import tempfile
import unittest
from run import stats, supervise, validate_output
from scale import diagnose
from report import pacing, read_rows, cpu_summary, growth, native_summary, NATIVE_METRICS
from workloads import block, cases, units


class Measurements(unittest.TestCase):
    def test_percentiles_and_missing(self):
        self.assertEqual(stats([]), dict(count=0, median=None, p95=None, maximum=None))
        self.assertEqual(stats([None, 1, 2, 3, 4])['median'], 2.5)
        self.assertEqual(stats(range(1, 31))['p95'], 29)
        self.assertEqual(stats([0])['maximum'], 0)

    def test_completion_requires_full_closed_output(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)/'output.jsonl'
            path.write_text(json.dumps(dict(type='scale', data={}))+'\n')
            with self.assertRaisesRegex(AssertionError, 'completion'):
                validate_output(path, 'native')
            path.write_text(json.dumps(dict(warmup=False))+'\n')
            with self.assertRaisesRegex(AssertionError, 'samples'):
                validate_output(path, 'warm')
            validate_output(path, 'cold')

    def test_workloads(self):
        matrix = list(cases())
        self.assertEqual(len(matrix), 96)
        self.assertEqual(len({c['id'] for c in matrix}), 96)
        for c in matrix:
            for b in c['blocks']:
                n = units(b['text'])
                for span in b['spans']+b['objects']:
                    self.assertLessEqual(span['start']+span['length'], n)
                if c['series'].startswith('spans-'):
                    self.assertEqual(sum(s['length'] for s in b['spans']), 8192)
                    self.assertEqual(n, 16384)
        for kind in ('prose', 'dense', 'mixed', 'long', 'objects'):
            self.assertEqual(units(block(kind, 1024)['text']), 1024)
            self.assertEqual(block(kind, 1024), block(kind, 1024))

    def test_timeout_partial_and_failure(self):
        with tempfile.TemporaryDirectory() as tmp:
            out, err = Path(tmp)/'out', Path(tmp)/'err'
            r = supervise([sys.executable, '-c', 'import time; print("partial",flush=True); time.sleep(5)'], out, err, timeout=.15)
            self.assertEqual(r['status'], 'timeout')
            self.assertEqual(out.read_text().strip(), 'partial')
            r = supervise([sys.executable, '-c', 'raise SystemExit(7)'], out, err)
            self.assertEqual(r['status'], 'failed')
            self.assertEqual(r['returncode'], 7)
            r = supervise([sys.executable, '-c', 'import time; a=bytearray(30000000); time.sleep(2)'], out, err, rss_limit=10000000)
            self.assertEqual(r['status'], 'rss-limit')

    def test_partial_aggregation(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)
            (path/'index.jsonl').write_text(json.dumps(dict(case='a', variant='zero', series='test', size=1,
                status='timeout', mode='warm', raw='raw.jsonl'))+'\n')
            rows = [dict(warmup=True, constructionMs=100), dict(warmup=False, constructionMs=2),
                    dict(warmup=False, constructionMs=4)]
            (path/'raw.jsonl').write_text('\n'.join(json.dumps(r) for r in rows)+'\n')
            summary = cpu_summary(path)[0]
            self.assertFalse(summary['modes']['warm']['complete'])
            self.assertEqual(summary['modes']['warm']['samples'], 2)
            self.assertEqual(summary['modes']['warm']['metrics']['constructionMs']['median'], 3)
            self.assertIsNone(summary['modes']['warm']['metrics']['metadataInvalidationMs']['median'])
            self.assertEqual(growth([summary]), [])

    def test_presentation_and_truncated_samples(self):
        scale = dict(clockResolutionNs=1, timestampUnitNs=1, presentationClock=1)
        rows = [dict(presentation='presented', presentedNs=i*16666667, refreshNs=16666667) for i in range(4)]
        self.assertTrue(pacing(rows, scale)['intervalTargetMet'])
        rows[-1]['presentedNs'] += 40000000
        self.assertFalse(pacing(rows, scale)['intervalTargetMet'])
        rows.append(dict(presentation='missing'))
        self.assertIsNone(pacing(rows, scale)['intervalTargetMet'])
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)/'partial.jsonl'
            path.write_text('{"ok": 1}\n{"unfinished":')
            self.assertEqual(read_rows(path), [dict(ok=1)])

    def test_scale(self):
        scale = dict(windowCoordinates=[800, 600], drawablePixels=[800, 600],
                     drawablePerWindowX=1, drawablePerWindowY=1, documentZoom=1)
        trace = 'wp_viewport#4.set_destination(640, 480)\nwp_fractional_scale_v1#5.preferred_scale(150)\nzwp_linux_buffer_params_v1#6.create(800, 600, 1, 0)'
        r = diagnose(scale, trace)
        self.assertTrue(r['mappingEstablished'])
        self.assertEqual(r['bufferPixelsPerSurfaceUnit'], [1.25, 1.25])
        self.assertFalse(diagnose(scale, '')['mappingEstablished'])
        scale['drawablePerWindowY'] = 2
        with self.assertRaises(AssertionError):
            diagnose(scale, trace)

    def test_native_window_qualification(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)
            (path/'index.jsonl').write_text(json.dumps(dict(case='a', variant='zero',
                api='opengl', scenario='stationary', repetition=0, status='completed', raw='raw.jsonl'))+'\n')
            scale = dict(clockResolutionNs=1, timestampUnitNs=1, presentationClock=1,
                         gpuElapsedSupported=False)
            frames = [dict({m:0 for m in NATIVE_METRICS}, frame=i, warmup=False,
                           layoutCalls=0, gpuElapsedMs=None, presentation='presented',
                           presentedNs=i*16666667, refreshNs=16666667) for i in range(4)]
            run = dict(type='run', warmupSeconds=2, measurementSeconds=5,
                       observedScenarioSeconds=6, captureRun=False, protocolDiagnostic=False)
            def write():
                (path/'raw.jsonl').write_text('\n'.join(json.dumps(r) for r in
                    [dict(type='scale', data=scale), *frames, run])+'\n')
            write()
            self.assertIsNone(native_summary(path)[0]['pacing']['intervalTargetMet'])
            run['observedScenarioSeconds'] = 7
            write()
            row = native_summary(path)[0]
            self.assertTrue(row['pacing']['intervalTargetMet'])
            self.assertIsNone(row['metrics']['gpuElapsedMs']['median'])
            self.assertIsNotNone(row['gpuTimingGap'])


if __name__ == '__main__':
    unittest.main()
