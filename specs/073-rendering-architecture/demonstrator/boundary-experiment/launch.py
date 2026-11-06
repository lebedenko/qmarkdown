#!/usr/bin/env python3
"""Launch the actual patched native viewer for owner inspection."""
import argparse
from pathlib import Path
import os
import subprocess

HERE = Path(__file__).resolve().parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('build',type=Path)
parser.add_argument('--backend',choices=['opengl','vulkan'],default='opengl')
parser.add_argument('--fixture',default='adjacent')
parser.add_argument('--manifest',type=Path,default=HERE.parent/'fixtures.json')
args = parser.parse_args()
raise SystemExit(subprocess.call([str((args.build/'patched/skia-compare').resolve()),str(args.manifest.resolve()),
    args.fixture,args.backend,'/tmp/073-boundary-interactive','--interactive'],
    env=dict(os.environ,SDL_VIDEODRIVER='wayland')))
