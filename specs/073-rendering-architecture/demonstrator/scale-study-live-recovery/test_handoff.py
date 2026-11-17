import json
import os
from pathlib import Path
import tempfile
import threading
import unittest
from unittest.mock import patch

import handoff
import run


class HandoffTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name)
        self.addCleanup(self.tmp.cleanup)

    def delayed_dump(self, value, stream, **kwargs):
        stream.write('{"scale":');stream.flush()
        self.started.set()
        if not self.release.wait(3):raise RuntimeError('test writer release timed out')
        stream.write('1.5}')

    def writer(self, save):
        try:save(self.path,{'scale':1.5})
        except BaseException as error:self.errors.append(error)

    def run_delayed(self, save, final_visible):
        self.path=self.root/'confirmation.json';self.started=threading.Event();self.release=threading.Event();self.errors=[]
        with patch.object(json,'dump',self.delayed_dump):
            thread=threading.Thread(target=self.writer,args=(save,));thread.start()
            try:
                self.assertTrue(self.started.wait(3))
                self.assertEqual(self.path.exists(),final_visible)
                if final_visible:
                    with self.assertRaises(json.JSONDecodeError):json.loads(self.path.read_text())
            finally:self.release.set();thread.join(3)
        self.assertFalse(thread.is_alive());self.assertEqual(self.errors,[])
        self.assertEqual(json.loads(self.path.read_text()),{'scale':1.5})

    def test_original_race_reproduces(self):
        def old_save(path,value):
            with path.open('x') as f:json.dump(value,f);f.flush();os.fsync(f.fileno())
        self.run_delayed(old_save,True)

    def test_paused_atomic_writer_is_not_visible(self):
        self.run_delayed(handoff.save,False)
        self.assertEqual(list(self.root.iterdir()),[self.path])

    def test_duplicate_preserves_original_bytes(self):
        path=self.root/'record.json';handoff.save(path,{'original':1});original=path.read_bytes()
        with self.assertRaises(FileExistsError):handoff.save(path,{'replacement':2})
        self.assertEqual(path.read_bytes(),original)
        self.assertEqual(list(self.root.iterdir()),[path])

    def test_two_concurrent_publishers(self):
        path=self.root/'record.json';barrier=threading.Barrier(2);link=os.link;errors=[]
        def racing_link(source,target):barrier.wait(timeout=3);link(source,target)
        def publish(value):
            try:handoff.save(path,value)
            except BaseException as error:errors.append(error)
        with patch.object(handoff.os,'link',racing_link):
            threads=[threading.Thread(target=publish,args=({'winner':i},)) for i in range(2)]
            for t in threads:t.start()
            for t in threads:t.join(3)
        self.assertTrue(all(not t.is_alive() for t in threads))
        self.assertEqual(len(errors),1);self.assertIsInstance(errors[0],FileExistsError)
        self.assertIn(json.loads(path.read_text()),[{'winner':0},{'winner':1}])
        self.assertEqual(list(self.root.iterdir()),[path])

    def test_serialization_failure_never_publishes(self):
        with self.assertRaises(TypeError):handoff.save(self.root/'record.json',{'bad':object()})
        self.assertEqual(list(self.root.iterdir()),[])

    def test_file_sync_failure_never_publishes(self):
        with patch.object(handoff.os,'fsync',side_effect=OSError('sync failed')):
            with self.assertRaises(OSError):handoff.save(self.root/'record.json',{'ok':1})
        self.assertEqual(list(self.root.iterdir()),[])

    def test_link_failure_never_publishes(self):
        with patch.object(handoff.os,'link',side_effect=OSError('link failed')):
            with self.assertRaises(OSError):handoff.save(self.root/'record.json',{'ok':1})
        self.assertEqual(list(self.root.iterdir()),[])

    def test_directory_sync_failure_keeps_complete_record(self):
        calls=0;sync=os.fsync
        def fail_second(fd):
            nonlocal calls
            calls+=1
            if calls==2:raise OSError('directory sync failed')
            sync(fd)
        with patch.object(handoff.os,'fsync',fail_second):
            with self.assertRaises(OSError):handoff.save(self.root/'record.json',{'ok':1})
        self.assertEqual(json.loads((self.root/'record.json').read_text()),{'ok':1})
        self.assertEqual(len(list(self.root.iterdir())),1)

    def test_unicode_roundtrip(self):
        value={'scale':1.25,'source':'لا אב','rows':[1,2,3]}
        handoff.save(self.root/'record.json',value)
        self.assertEqual(json.loads((self.root/'record.json').read_text()),value)

    def setup_schedule(self):
        cells=[dict(id=str(680+i),kind='live',command=['fatal-launch-sentinel']) for i in range(10)]
        handoff.save(self.root/'schedule.json',cells)
        handoff.save(self.root/'freeze.json',dict(identities={},scheduleHash=run.study.sha(self.root/'schedule.json'),maxLaunches=10,priorAutomatedSeconds=240))
        return cells

    def test_ten_window_bound_and_wrong_cell(self):
        cells=self.setup_schedule();journal=run.Journal(self.root)
        with patch.object(run.study.subprocess,'Popen',side_effect=AssertionError('unexpected launch')):
            with self.assertRaises(RuntimeError):journal.start(dict(id='000',kind='fresh',command=[]))
            for cell in cells:journal.start(cell)
            with self.assertRaises(RuntimeError):journal.start(cells[0])
        self.assertEqual(len(journal.started),10);journal.lock.close()

    def test_approval_gate_fails_without_launch(self):
        self.setup_schedule()
        with patch.object(run.study.subprocess,'Popen',side_effect=AssertionError('unexpected launch')):
            with self.assertRaises(FileNotFoundError):run.require_approval(self.root)
            handoff.save(self.root/'approval.json',dict(approved=True,scope='wrong scope',scheduleHash='wrong'))
            with self.assertRaises(RuntimeError):run.require_approval(self.root)


if __name__=='__main__':unittest.main()
