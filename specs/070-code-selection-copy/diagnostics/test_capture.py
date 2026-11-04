import json
import os
from pathlib import Path
import pty
import select
import signal
import subprocess
import sys
import tempfile
import termios
import unittest

from capture import describe, record


class CaptureTest(unittest.TestCase):
    def test_literals(self):
        data = '    while\tbeta\n\n  中文 😀 e\u0301 tail\r\n'.encode()
        result = record(b'\x1b[200~' + data + b'\x1b[201~')
        self.assertEqual(bytes.fromhex(result['payload']['hex']), data)
        self.assertEqual(json.loads(json.dumps(result))['payload'], describe(data))
        self.assertTrue(result['bracketed_paste'])
        self.assertFalse(record(b'\x1b[200~unfinished')['bracketed_paste'])

    def test_comparator(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            data = '  x\ty\n中文 😀 e\u0301'.encode()
            (root / 'expected').write_bytes(data)
            rows = [dict(pid=42, action=1, stage=stage, **describe(data))
                    for stage in ['partial', 'selectedText', 'writer', 'readback']]
            (root / 'trace').write_text('\n'.join(json.dumps(r) for r in rows))
            for name in ['local', 'remote']:
                (root / name).write_text(json.dumps(dict(action='42:1', **record(data))))
            command = [sys.executable, str(Path(__file__).with_name('compare.py')),
                       str(root / 'expected'), str(root / 'trace'), '--pid', '42', '--action', '1',
                       '--local', str(root / 'local'), '--remote', str(root / 'remote')]
            self.assertEqual(subprocess.run(command, capture_output=True).returncode, 0)
            rows[2]['hex'] = b'wrong'.hex()
            (root / 'trace').write_text('\n'.join(json.dumps(r) for r in rows))
            failed = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(failed.returncode, 1)
            self.assertIn('earliest divergent stage: writer', failed.stdout)

    def test_receiver_termination_restores(self):
        master, slave = pty.openpty()
        old = termios.tcgetattr(slave)
        with tempfile.TemporaryDirectory() as directory:
            def terminal():
                import fcntl
                os.setsid()
                fcntl.ioctl(slave, termios.TIOCSCTTY, 0)
            proc = subprocess.Popen([sys.executable, str(Path(__file__).with_name('capture.py')),
                                     str(Path(directory) / 'cancel.json'), '--action', 'cancel'],
                                    stdin=slave, stdout=slave, stderr=slave, preexec_fn=terminal)
            try:
                received = b''
                while b'\x1b[?2004h' not in received:
                    self.assertTrue(select.select([master], [], [], 5)[0])
                    received += os.read(master, 4096)
                proc.send_signal(signal.SIGTERM)
                self.assertEqual(proc.wait(timeout=5), 128 + signal.SIGTERM)
                self.assertEqual(termios.tcgetattr(slave), old)
            finally:
                if proc.poll() is None:
                    proc.kill()
                    proc.wait()
                os.close(master)
                os.close(slave)

    def test_real_receiver_and_restore(self):
        master, slave = pty.openpty()
        old = termios.tcgetattr(slave)
        with tempfile.TemporaryDirectory() as directory:
            out = Path(directory) / 'paste.json'
            def terminal():
                import fcntl
                os.setsid()
                fcntl.ioctl(slave, termios.TIOCSCTTY, 0)
            proc = subprocess.Popen([sys.executable, str(Path(__file__).with_name('capture.py')),
                                     str(out), '--action', 'test-1'],
                                    stdin=slave, stdout=slave, stderr=slave, preexec_fn=terminal)
            try:
                received = b''
                while b'\x1b[?2004h' not in received:
                    self.assertTrue(select.select([master], [], [], 5)[0])
                    received += os.read(master, 4096)
                payload = '  x\ty\n\n中文 😀 e\u0301\r\n'.encode()
                os.write(master, b'\x1b[200~' + payload + b'\x1b[201~\x1d')
                self.assertEqual(proc.wait(timeout=5), 0)
                self.assertEqual(json.loads(out.read_text())['payload']['hex'], payload.hex())
                self.assertEqual(termios.tcgetattr(slave), old)
            finally:
                if proc.poll() is None:
                    proc.kill()
                    proc.wait()
                os.close(master)
                os.close(slave)


if __name__ == '__main__':
    unittest.main()
