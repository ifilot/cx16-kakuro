"""Build the static game site with the official, checksum-pinned r49 emulator."""
from hashlib import sha256
from pathlib import Path
import json
import shutil
import urllib.request
from zipfile import ZipFile, ZIP_DEFLATED

ROOT=Path(__file__).resolve().parents[1]
ARCHIVE='x16emu_wasm-r49.zip'
URL=f'https://github.com/X16Community/x16-emulator/releases/download/r49/{ARCHIVE}'
DIGEST='9711036cf5b33504815b278fa872b8ec20aef9cb609fd7b17f3685d91312628f'


def main():
    cache=ROOT/'build/site-cache';cache.mkdir(parents=True,exist_ok=True)
    archive=cache/ARCHIVE
    if not archive.exists():
        print('Downloading official X16 WebAssembly emulator r49…',flush=True)
        with urllib.request.urlopen(URL,timeout=60) as response:
            data=response.read()
        if sha256(data).hexdigest()!=DIGEST:raise ValueError('Emulator archive checksum mismatch')
        archive.write_bytes(data)
    if sha256(archive.read_bytes()).hexdigest()!=DIGEST:
        raise ValueError(f'Emulator archive checksum mismatch: remove {archive} and retry')
    output=ROOT/'build/site'
    shutil.rmtree(output,ignore_errors=True)
    shutil.copytree(ROOT/'site',output)
    (output/'emulator').mkdir()
    with ZipFile(archive) as package:
        for name in ('x16emu.js','x16emu.wasm','x16emu.data'):
            (output/'emulator'/name).write_bytes(package.read(name))
    (output/'game').mkdir()
    with ZipFile(ROOT/'build/CX16-KAKURO.ZIP') as package:
        resources=[]
        for name in package.namelist():
            if '/' in name or '\\' in name:raise ValueError('Distribution must contain flat filenames')
            (output/'game'/name).write_bytes(package.read(name));resources.append(name)
    # Always publish fresh puzzle data, never the developer's saved progress.
    import sys
    sys.path.insert(0,str(ROOT/'assets/scripts'))
    from create_puzzle import encode_puzzle
    import struct
    puzzles=[encode_puzzle(f'{i:03}.puz') for i in range(1,97)]
    offsets=[];position=194
    for data in puzzles:offsets.append(position);position+=len(data)
    (output/'game/PUZZLE.DAT').write_bytes(struct.pack('<HH',0xA000,96)+struct.pack('<96H',*offsets)+b''.join(puzzles))
    (output/'game/manifest.json').write_text(json.dumps({'resources':sorted(resources)},indent=2)+'\n')
    with ZipFile(output/'CX16-KAKURO.ZIP','w',compression=ZIP_DEFLATED) as package:
        for name in resources:package.write(output/'game'/name,name)
    shutil.copy2(ROOT/'assets/backgrounds/start.png',output/'courtyard.png')
    (output/'.nojekyll').touch()
    info=json.loads((ROOT/'src/BUILDINFO.json').read_text())
    (output/'build.json').write_text(json.dumps({'version':info['version'],'commit':info['commit'],'emulator':'r49'},indent=2)+'\n')
    print(f'Site ready: {output} ({len(resources)} runtime files)')


if __name__=='__main__':main()
