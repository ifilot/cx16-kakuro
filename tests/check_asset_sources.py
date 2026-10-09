"""Check a fresh build and that edits to live source sheets reach game assets.

All edits and builds use a temporary checkout; saved puzzle progress is untouched.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]


def main():
    with tempfile.TemporaryDirectory(prefix='kakuro-sources-') as directory:
        root=Path(directory)
        shutil.copytree(ROOT/'assets',root/'assets',ignore=shutil.ignore_patterns('__pycache__'))
        shutil.copytree(ROOT/'src',root/'src',ignore=shutil.ignore_patterns(
            '*.DAT','*.PRG','*.ZSM','*.BIN','*.o','*.map','*.sym','sfx.h','tile_layout.h','BUILDINFO.json'))
        for name in ('Makefile','VERSION'):shutil.copy2(ROOT/name,root/name)
        env=dict(os.environ,GIT_DIR=str(ROOT/'.git'),GIT_WORK_TREE=str(root),PYTHONDONTWRITEBYTECODE='1')
        def build():
            result=subprocess.run(['make'],cwd=root,env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            assert result.returncode==0,result.stdout
            return result.stdout
        def read(name):return (root/'src'/name).read_bytes()
        build()
        assert (root/'src/KAKURO.PRG').is_file()
        assert not any((root/'src'/name).exists() for name in ('TILES.DAT','MTILES.DAT','SDIGITS.DAT','FONT16.DAT'))
        for name in ('GTILES.DAT','FONT8.DAT','JUI.DAT','GQUIT.DAT','SPLASH0.DAT','SPLASH1.DAT'):
            assert read(name)==(ROOT/'src'/name).read_bytes(),name
        atlas=read('GTILES.DAT');font=read('FONT8.DAT');menu=read('JUI.DAT')
        # Alter the first blocked-cell pixel. Only that cell's atlas tile may change.
        sheet=root/'assets/tiles/cells.png'
        with Image.open(sheet) as source:
            image=source.convert('RGB');old=image.getpixel((0,0))
            image.putpixel((0,0),(136,102,102) if old!=(136,102,102) else (68,51,51));image.save(sheet)
        output=build()
        assert 'create_playfield.py' in output and 'cl65 ' in output,output
        changed=read('GTILES.DAT')
        assert changed[:128]==atlas[:128] and changed[256:]==atlas[256:]
        assert changed[128:256]!=atlas[128:256]
        assert read('FONT8.DAT')==font and read('JUI.DAT')==menu
        # Alter one bit in ASCII H: both packed font and rendered controls must update.
        sheet=root/'assets/tiles/ui-charmap.png'
        with Image.open(sheet) as source:
            image=source.convert('RGBA');x,y=(72-32)%16*8,(72-32)//16*8
            pixel=image.getpixel((x,y));image.putpixel((x,y),(*pixel[:3],0 if pixel[3]>150 else 255));image.save(sheet)
        output=build()
        for script in ('create_fontmap.py','create_menu.py','create_playfield.py','create_documents.py'):
            assert script in output,output
        assert read('FONT8.DAT')!=font
        assert read('JUI.DAT')!=menu
        assert read('GTILES.DAT')!=changed
        print('PASS: fresh source-only build; cell and charmap edits propagate through make into game assets.')


if __name__=='__main__':main()
