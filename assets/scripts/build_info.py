"""Record reproducible build inputs; keep the timestamp stable for unchanged inputs."""
import json
import hashlib
from pathlib import Path
import subprocess
import sys
from datetime import datetime, timezone
output=Path(sys.argv[1])
compiler=sys.argv[2]
flags=sys.argv[3]
root=Path(__file__).resolve().parents[2]
def command(args):
    return subprocess.check_output(args,cwd=root,stderr=subprocess.STDOUT,text=True).strip()
info={'version':(root/'VERSION').read_text().strip(),
      'commit':command(['git','rev-parse','HEAD']),
      'tree':'modified' if command(['git','status','--porcelain']) else 'clean',
      'compiler':command([compiler,'--version']).splitlines()[0],
      'flags':flags,'target':'cx16','linker':'kakuro.cfg'}
inputs=hashlib.sha256()
paths=[p for p in (root/'src').iterdir() if (p.suffix in ('.c','.h','.s','.lib','.cfg') and p.name!='sfx.h') or p.name in ('Makefile','ABOUT.TXT','HELP.TXT')]
paths+=list((root/'assets/scripts').glob('*.py'))
paths+=list((root/'assets/sound').glob('*.ZSM'))+list((root/'assets/sound').glob('*.BIN'))
paths+=[root/'assets/sound/sfx.h']+list((root/'assets/puzzles').glob('*.puz'))+list((root/'assets/tiles').glob('*.png'))
for path in sorted(paths):
    inputs.update(str(path.relative_to(root)).encode());inputs.update(path.read_bytes())
info['inputs']=inputs.hexdigest()
old=json.loads(output.read_text()) if output.exists() else {}
if {k:v for k,v in old.items() if k!='built'}!=info:
    info['built']=datetime.now(timezone.utc).strftime('%Y-%m-%d %H:%M UTC')
    output.write_text(json.dumps(info,indent=2)+'\n')
