#!/usr/bin/env python3
"""Capture one paste as bytes, never execute it. Finish with Ctrl-] after paste."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import select
import signal
import termios
import tty


def describe(data):
    return dict(utf8_text=data.decode('utf-8', errors='replace'),
                utf8_valid=valid_utf8(data), byte_length=len(data),
                hex=data.hex(), sha256=hashlib.sha256(data).hexdigest())


def valid_utf8(data):
    try:
        data.decode('utf-8')
        return True
    except UnicodeDecodeError:
        return False


def record(wire):
    start, end = b'\x1b[200~', b'\x1b[201~'
    framed = wire.startswith(start) and wire.endswith(end)
    return dict(wire=describe(wire), bracketed_paste=framed,
                payload=describe(wire[len(start):-len(end)] if framed else wire),
                newline_conversion='disabled: raw termios (including ICRNL/INLCR/OPOST)',
                finish='Ctrl-] excluded; framing removed only for one exact outer pair')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    parser.add_argument('--action', required=True)
    args = parser.parse_args()
    # Reserve output before changing terminal state; never overwrite evidence.
    with args.output.open('x') as output:
        fd = os.open('/dev/tty', os.O_RDWR)
        old = termios.tcgetattr(fd)
        data = bytearray()
        def interrupted(signum, frame):
            raise SystemExit(128 + signum)
        handlers = {sig: signal.getsignal(sig) for sig in (signal.SIGTERM, signal.SIGHUP)}
        try:
            for sig in handlers:
                signal.signal(sig, interrupted)
            os.write(fd, b'Paste once; press Ctrl-] to finish (Ctrl-C cancels).\r\n')
            tty.setraw(fd)
            os.write(fd, b'\x1b[?2004h')
            while True:
                if not select.select([fd], [], [], 120)[0]:
                    raise TimeoutError('No input for 120 seconds')
                chunk = os.read(fd, 1)
                if chunk == b'\x1d':
                    break
                if chunk == b'\x03':
                    raise KeyboardInterrupt
                if not chunk:
                    raise EOFError('Terminal closed')
                data.extend(chunk)
        finally:
            os.write(fd, b'\x1b[?2004l')
            termios.tcsetattr(fd, termios.TCSANOW, old)
            os.close(fd)
            for sig, handler in handlers.items():
                signal.signal(sig, handler)
        output.write(json.dumps(dict(action=args.action, **record(bytes(data))), ensure_ascii=True) + '\n')


if __name__ == '__main__':
    main()
