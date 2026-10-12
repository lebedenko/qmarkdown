import hashlib,json,re,subprocess
from pathlib import Path
root=Path('/tmp/qmarkdown-033')
base=root/'specs/032-v1.2-integration'
d=json.loads((base/'run-evidence.json').read_text())
for name,digest in d['evidenceSha256'].items(): assert hashlib.sha256((base/name).read_bytes()).hexdigest()==digest,name
files=['README.md','CHANGELOG.md','docs/release-notes-1.2.0.md','docs/release-install.md','docs/arch-install.md','docs/code-block-actions.md','specs/README.md','specs/roadmap.md']+[str(p.relative_to(root)) for p in (root/'specs/033-stable-v1.2-release').glob('*.md')]
for name in files:
 p=root/name; t=re.sub(r'```.*?```','',p.read_text(),flags=re.S)
 for target in re.findall(r'\]\(([^)]+)\)',t):
  if ':' in target or target.startswith('#'): continue
  assert (p.parent/target.split('#')[0]).exists(),(name,target)
subprocess.run(['git','diff','--check'],cwd=root,check=True)
print('PASS: 95 retained evidence hashes, current prose local links and whitespace')
