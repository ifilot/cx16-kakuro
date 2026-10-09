"""Check journal documents in the live CX16 emulator without saving game progress."""
import argparse, re, shutil, sys, tempfile
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT.parent/'x16-emulator/api/python'))
from x16agent import X16Agent
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--screenshots',type=Path)
args=parser.parse_args()
symbols={n:int(a,16) for a,n in re.findall(r'al ([0-9A-Fa-f]+) \.(\S+)',(ROOT/'src/KAKURO.sym').read_text())}
with tempfile.TemporaryDirectory(prefix='kakuro-documents-') as temp:
    runtime=Path(temp)
    captures=args.screenshots.resolve() if args.screenshots else runtime/'screenshots'
    captures.mkdir(parents=True,exist_ok=True)
    for pattern in ('*.DAT','*.ZSM', '*.BIN','KAKURO.PRG'):
        for source in (ROOT/'src').glob(pattern):shutil.copy2(source,runtime/source.name)
    with X16Agent(ROOT.parent/'x16-emulator/build/x16emu',ROOT.parent/'emulator/rom.bin',fsroot=runtime,options=['-prg',str(runtime/'KAKURO.PRG'),'-run']) as emu:
        def advance(n=16):
            assert emu.advance(n)['reason']=='target'
        def key(name):
            emu.request('key',key=name,down=True);advance(8)
            emu.request('key',key=name,down=False);advance(32)
        def ready(name):
            emu.request('breakpoint',address=symbols[name])
            for _ in range(4):
                result=emu.advance(100000000,unit='cycles')
                if result['reason']=='breakpoint':break
            emu.request('breakpoint',action='clear');assert result['reason']=='breakpoint',result
        def value(name):return int.from_bytes(emu.read_memory(symbols[name],2),'little')
        def move(x,y):
            for _ in range(8):
                pos=emu.read_memory(2,4);px=int.from_bytes(pos[:2],'little');py=int.from_bytes(pos[2:],'little')
                if (px,py)==(x,y):return
                emu.request('mouse',dx=max(-255,min(255,x-px)),dy=max(-255,min(255,y-py)));advance()
            raise AssertionError('Mouse position')
        def click(x,y):
            move(x,y);emu.request('mouse',buttons=1);advance(12)
            emu.request('mouse',buttons=0);advance(120)
        def check_text(name):
            records=(ROOT/f'src/D{name}T.DAT').read_bytes()
            top=value('_docview_top_line');total=value('_docview_line_count')
            assert total==int.from_bytes(records[:2],'little')
            for row in range(18):
                line=top+row;expected=bytearray()
                record=records[2+line*64:2+(line+1)*64]
                for col in range(60):expected.extend((record[col]-32,record[60]) if line<total else (0,0))
                assert emu.read_memory(0x18000+(14+row*2)*256+18,120,space='vram')==expected
        advance(180);key('Return');ready('_menu_handle_mouse')
        for name,x in [('HELP',106),('ABOUT',354)]:
            click(x,432);ready('_docview_handle_key');advance(2);check_text(name)
            assert value('_docview_top_line')==0
            move(320,88);advance(2)
            path=captures/f'cx16-kakuro-journal-{name.lower()}.png'
            emu.screenshot(path)
            with Image.open(path) as image:
                assert len(image.convert('RGB').getcolors(640*480))==4
            emu.request('breakpoint',address=symbols['_docview_render'])
            emu.request('key',key='Down',down=True)
            result=emu.advance(1000000,unit='cycles');assert result['reason']=='breakpoint'
            start=result['state']['cycles'];emu.request('breakpoint',action='clear')
            emu.advance(1,unit='instructions')
            emu.request('breakpoint',address=symbols['_docview_handle_key'])
            result=emu.advance(1000000,unit='cycles');assert result['reason']=='breakpoint'
            elapsed=result['state']['cycles']-start
            assert elapsed<133334,elapsed
            print(f'{name}: scroll redraw {elapsed} cycles ({elapsed/8000:.2f} ms)')
            emu.request('breakpoint',action='clear');emu.request('key',key='Down',down=False);advance(16)
            assert value('_docview_top_line')==1;check_text(name)
            key('Up');assert value('_docview_top_line')==0
            key('PageDown');assert value('_docview_top_line')==18
            key('PageUp');assert value('_docview_top_line')==0
            key('End');assert value('_docview_top_line')==value('_docview_line_count')-18;check_text(name)
            key('Down');assert value('_docview_top_line')==value('_docview_line_count')-18
            key('Home');assert value('_docview_top_line')==0
            click(550,444);assert value('_docview_top_line')==1
            click(470,444);assert value('_docview_top_line')==0
            emu.request('mouse',wheel=-1);advance(32);assert value('_docview_top_line')==3
            emu.request('mouse',wheel=1);advance(32);assert value('_docview_top_line')==0
            click(580,390);assert value('_docview_top_line')==18;check_text(name)
            key('Escape');ready('_menu_handle_mouse')
        click(106,432);ready('_docview_handle_key');key('Escape');ready('_menu_handle_mouse')
        print('PASS: both pages, text, palette, scrolling keys, limits, wheel, scrollbar, controls and return.')
