#!/usr/bin/env python3
"""Offline fail-closed tests; children here are Python fixtures, never viewers."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
import unittest
from unittest.mock import patch
import study


class StudyTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name)
        self.addCleanup(self.tmp.cleanup)

    def output(self, scale=1.25):
        return [dict(name='DP-1',enabled=True,scale=scale,serial='s')]

    def test_confirm_scale_and_output(self):
        self.assertEqual(study.output_scale(self.output(),1.25)['name'],'DP-1')
        for outputs,scale in [(self.output(),1),([dict(name='DP-2',enabled=True,scale=1)],1),
                              (self.output()+self.output(),1.25)]:
            with self.assertRaises((ValueError,RuntimeError)):study.output_scale(outputs,scale)
        with self.assertRaises(RuntimeError):study.output_scale(self.output(),1.25,dict(name='DP-1',serial='other'))

    def test_no_scale_override(self):
        with patch.dict(os.environ,{'QT_SCALE_FACTOR':'4','QT_SCREEN_SCALE_FACTORS':'DP-1=4','QT_FONT_DPI':'300'}):
            env=study.environment(dict(api='opengl'))
        self.assertNotIn('QT_SCALE_FACTOR',env);self.assertNotIn('QT_FONT_DPI',env)

    def test_schedule(self):
        manifest={'fixtures':[{'id':'document'}]+[{'id':str(n)} for n in range(14)]}
        study.save(self.root/'fixtures.json',manifest)
        cells=study.schedule(self.root,self.root/'run')
        self.assertEqual(len(cells),690);self.assertEqual(len({r['id'] for r in cells}),690)
        self.assertEqual(sum(c['kind']=='live' for c in cells),10)
        self.assertEqual(sum('--motion-replay' in c['command'] for c in cells),600)
        self.assertEqual(cells[0]['fixture'],'document');self.assertEqual(cells[16]['cycle'],2)
        self.assertEqual(cells[170]['scale'],1.25)

    def test_deadlines_and_rss(self):
        self.assertEqual(study.limits(21,0,20,0),'operation deadline')
        self.assertEqual(study.limits(121,0,200,0),'absolute 120-second deadline')
        self.assertIsNone(study.limits(121,0,200,0,False))
        self.assertIsNone(study.limits(1,0,20,4*study.GIB))
        self.assertIsNotNone(study.limits(1,0,20,4*study.GIB+1))

    def test_output_protocol(self):
        trace='wl_output#4.name("DP-1")\nwl_surface#8.enter(wl_output#4)'
        self.assertEqual(study.protocol_output(trace)['surfaceEnters'],[('8','4')])
        for bad in ('',trace.replace('DP-1','DP-2'),trace.replace('wl_output#4)','wl_output#9)')):
            with self.assertRaises(RuntimeError):study.protocol_output(bad)

    def test_scale_change_without_buffer_change(self):
        # SDL drawable ratio may stay 1. Surface preference must still be recorded.
        trace='wl_output#4.name("DP-1")\nwl_surface#8.enter(wl_output#4)\n'
        a=study.protocol_output(trace+'wp_fractional_scale_v1#9.preferred_scale(150)')
        b=study.protocol_output(trace+'wp_fractional_scale_v1#9.preferred_scale(180)')
        self.assertNotEqual(a,b)
        self.assertEqual(a['fractionalScale120ths'],[150]);self.assertEqual(b['fractionalScale120ths'],[180])

    def test_duplicate_and_interrupted_resume(self):
        cell=dict(id='000',command=['fatal-launch-sentinel'])
        j=study.Journal(self.root);j.start(cell)
        with self.assertRaises(RuntimeError):j.start(cell)
        j.lock.close();resumed=study.Journal(self.root)
        self.assertIn('000',resumed.started);self.assertEqual(resumed.seconds,120)
        with self.assertRaises(RuntimeError):resumed.start(cell)
        resumed.lock.close()

    def test_completed_resume_integrity(self):
        cell=dict(id='000',command=['fatal-launch-sentinel'])
        j=study.Journal(self.root);j.start(cell)
        folder=self.root/'attempts/000';folder.mkdir(parents=True)
        result=dict(status='passed',seconds=1,captures={})
        study.save(folder/'result.json',result);j.finish(cell,result);j.lock.close()
        j=study.Journal(self.root);self.assertEqual(j.seconds,1);j.lock.close()
        (folder/'result.json').write_text('{}')
        with self.assertRaises(RuntimeError):study.Journal(self.root)

    def test_malformed_journal(self):
        (self.root/'attempts.jsonl').write_text('{')
        with self.assertRaises(RuntimeError):study.Journal(self.root)

    def test_immutable_write(self):
        study.save(self.root/'capture.json',{})
        with self.assertRaises(FileExistsError):study.save(self.root/'capture.json',{})

    def test_frozen_input_rejection(self):
        p=self.root/'binary';p.write_text('original')
        study.save(self.root/'schedule.json',[])
        study.save(self.root/'freeze.json',dict(identities={str(p):study.sha(p)},scheduleHash=study.sha(self.root/'schedule.json')))
        study.verify_identities(self.root);p.write_text('changed')
        with self.assertRaises(RuntimeError):study.verify_identities(self.root)

    def test_malformed_child_record(self):
        cell=dict(id='000',config=dict(api='opengl'),command=[sys.executable,'-c','print("not-json")'])
        p=study.Process(cell,self.root)
        try:
            until=time.monotonic()+3
            while not p.eof and time.monotonic()<until:p.pump();time.sleep(.01)
            self.assertTrue(p.errors)
        finally:p.stop()

    def test_startup_progress_does_not_extend_deadline(self):
        cell=dict(id='000',config=dict(api='opengl'),command=[sys.executable,'-c',
            'print(\'{"stage":"capture"}\',flush=True)'])
        p=study.Process(cell,self.root);deadline=p.deadline
        try:
            until=time.monotonic()+3
            while not p.eof and time.monotonic()<until:p.pump();time.sleep(.01)
            self.assertEqual(deadline,p.deadline);self.assertFalse(p.ready)
        finally:p.stop()

    def test_missing_confirmation_never_launches(self):
        study.save(self.root/'freeze.json',dict(identities={},scheduleHash=''))
        with patch.object(study,'verify_identities',return_value={'original':{}}),patch.object(study,'Process') as launch:
            with self.assertRaises(FileNotFoundError):study.fresh(self.root,1)
            launch.assert_not_called()


if __name__=='__main__':unittest.main()
