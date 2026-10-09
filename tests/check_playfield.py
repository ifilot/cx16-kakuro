"""Exercise journal gameplay, all five grid sizes, entry/check/erase and dialogs."""
import argparse
from pathlib import Path
import re
import shutil
import sys
import tempfile
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--screenshots',type=Path)
    args=parser.parse_args()
    sys.path.insert(0,str(ROOT.parent/'x16-emulator/api/python'))
    from x16agent import X16Agent
    symbols={name:int(address,16) for address,name in re.findall(r'al ([0-9A-Fa-f]+) \.(\S+)',(ROOT/'src/KAKURO.sym').read_text())}
    with tempfile.TemporaryDirectory(prefix='kakuro-playfield-') as directory:
        runtime=Path(directory)
        captures=args.screenshots.resolve() if args.screenshots else runtime/'screenshots'
        captures.mkdir(parents=True,exist_ok=True)
        for pattern in ('*.DAT','*.ZSM', '*.BIN','*.TXT','KAKURO.PRG'):
            for source in (ROOT/'src').glob(pattern):shutil.copy2(source,runtime/source.name)
        with (runtime/'emulator.log').open('w') as log,X16Agent(
            ROOT.parent/'x16-emulator/build/x16emu',ROOT.parent/'emulator/rom.bin',fsroot=runtime,
            options=['-prg',str(runtime/'KAKURO.PRG'),'-run'],stderr=log) as emu:
            def variable(name,length=1):return int.from_bytes(emu.read_memory(symbols[name],length),'little')
            def advance(frames):
                result=emu.advance(frames)
                assert result['reason']=='target',result
            def ready(game=False):
                emu.request('breakpoint',address=symbols['_puzzle_handle_mouse' if game else '_menu_handle_mouse'])
                for _ in range(5):
                    result=emu.advance(100000000,unit='cycles')
                    if result['reason']=='breakpoint':break
                emu.request('breakpoint',action='clear')
                assert result['reason']=='breakpoint',result
                if game:
                    assert emu.read_memory(0x9f2d)[0]==5
                    assert emu.read_memory(0x9f34)[0]==0x12
                    assert emu.read_memory(0x1fa00,10,space='vram')==(ROOT/'src/JPALET.DAT').read_bytes()
            def key(name,settle=16):
                emu.request('key',key=name,down=True);advance(8)
                emu.request('key',key=name,down=False);advance(settle)
            def move(x,y,game=False):
                for _ in range(8):
                    position=emu.read_memory(2,4)
                    px=int.from_bytes(position[:2],'little');py=int.from_bytes(position[2:],'little')
                    if (px,py)==(x,y):return
                    emu.request('mouse',dx=max(-255,min(255,x-px)),dy=max(-255,min(255,y-py)))
                    advance(8);ready(game)
                raise AssertionError('Mouse did not reach target')
            def click(x,y,game=False):
                move(x,y,game)
                emu.request('mouse',buttons=1);advance(8)
                emu.request('mouse',buttons=0);advance(100)
            def capture(name):
                atlas=(ROOT/'src/GTILES.DAT').read_bytes()
                for start in range(0,len(atlas),16384):
                    assert emu.read_memory(0x13000+start,min(16384,len(atlas)-start),space='vram')==atlas[start:start+16384]
                path=captures/(name+'.png');assert emu.screenshot(path)['captured']
                with Image.open(path) as image:
                    assert image.size==(640,480)
                    count=len(image.getcolors(640*480))
                    if count!=4:
                        print('Sprite:',emu.read_memory(0x1fc00,16,space='vram').hex())
                        print('Cursor bytes match:',emu.read_memory(0x1e000,256,space='vram')==(ROOT/'src/JCURSOR.DAT').read_bytes())
                    assert count==4
            def open_puzzle(number):
                page=(number-1)//24
                while variable('_menu_page')!=page:
                    click(568 if variable('_menu_page')<page else 440,432);ready()
                slot=(number-1)%24;col=slot%6;row=slot//6
                click(128+col*72+(24 if col>=3 else 0),136+row*56)
                ready(True)
                assert variable('_current_puzzle_id')==number-1
            def quit_game():
                key('Escape');key('Y',180);ready()
            def status(number):
                raw=emu.read_memory(0,8192,space='banked_ram',bank=2)
                start=int.from_bytes(raw[number*2:number*2+2],'little')
                shape=raw[start];known=start+1+((shape>>4)*(shape&15)+1)//2
                return raw[known+1+raw[known]]
            advance(300);key('Return',180);ready()
            # Select one representative of every supported board size.
            data=(ROOT/'src/PUZZLE.DAT').read_bytes()[2:]
            representatives={}
            for number in range(1,97):
                start=int.from_bytes(data[number*2:number*2+2],'little')
                representatives.setdefault(data[start]>>4,number)
            for size,number in sorted(representatives.items()):
                open_puzzle(number)
                capture(f'game-{size}x{size}')
                cells=emu.read_memory(variable('_puzzledata',2),size*size)
                tilemap=emu.read_memory(0x1e200,4096,space='vram')
                rating=(status(number)>>6)+1
                difficulty_tiles=[int.from_bytes(tilemap[(11*64+28+i)*2:(11*64+29+i)*2],'little')&1023 for i in range(5)]
                assert difficulty_tiles==[349]*rating+[350]*(5-rating)
                for index,value in enumerate(cells):
                    row,col=divmod(index,size)
                    for flag,step,down in ((0x40,1,0),(0x80,size,1)):
                        if not value&flag:continue
                        total=0;next_index=index+step
                        while next_index<len(cells) and cells[next_index]&15:
                            total+=cells[next_index]&15;next_index+=step
                        y=variable('_offset_y')+row*2+down
                        x=variable('_offset_x')+col*2+(1-down)
                        actual=int.from_bytes(tilemap[(y*64+x)*2:(y*64+x)*2+2],'little')&1023
                        assert actual==197+(total-2)*2+down,(number,row,col,actual,total)
                if size!=6:
                    quit_game();continue
                rows=variable('_puzzlerows');cols=variable('_puzzlecols')
                cells=emu.read_memory(variable('_puzzledata',2),rows*cols)
                user=variable('_userdata',2)
                editable=[i for i,c in enumerate(cells) if not c&0x30]
                assert variable('_tiles_incorrect',2)==len(editable)
                index=editable[0];r,c=divmod(index,cols)
                x=variable('_offset_x')*16+c*32+16
                y=variable('_offset_y')*16+r*32+16
                # Sample complete game iterations while a real PS/2 hover
                # arrives, including keyboard, clock and sound processing.
                position=emu.read_memory(2,4)
                px=int.from_bytes(position[:2],'little');py=int.from_bytes(position[2:],'little')
                emu.request('mouse',dx=x-px,dy=y-py)
                entry=symbols['_puzzle_handle_mouse']
                total=peak=0
                while total<400000:
                    emu.request('breakpoint',action='clear')
                    first=emu.advance(1,unit='instructions')['cycles']
                    emu.request('breakpoint',address=entry)
                    result=emu.advance(10000000,unit='cycles')
                    assert result['reason']=='breakpoint',result
                    cost=first+result['cycles'];total+=cost;peak=max(peak,cost)
                emu.request('breakpoint',action='clear')
                assert peak<=133334,peak
                print(f'Game hover: {peak:,} cycles ({peak/8000:.2f} ms at 8 MHz).')
                move(x,y,True)
                correct=cells[index]&15;wrong=correct%9+1
                key(str(wrong));ready(True)
                assert emu.read_memory(user+index)[0]&15==wrong
                click(518,304,True);ready(True)
                assert variable('_gamestate')&4
                capture('game-checking')
                move(x,y,True);key(str(correct));ready(True)
                assert variable('_tiles_incorrect',2)==len(editable)-1
                key('Backspace');ready(True)
                assert emu.read_memory(user+index)[0]==0
                assert variable('_tiles_incorrect',2)==len(editable)
                move(16,16,True)
                before=emu.read_memory(user,rows*cols)
                key('9');ready(True)
                assert emu.read_memory(user,rows*cols)==before
                # Canceling a quit restores the board overlay exactly.
                before=emu.read_memory(0x1e200,4096,space='vram')
                bitmap_before=[emu.read_memory((192+y)*160+32,96,space='vram') for y in range(96)]
                key('Escape');capture('game-quit-dialog');key('N');ready(True)
                assert bitmap_before==[emu.read_memory((192+y)*160+32,96,space='vram') for y in range(96)]
                after=emu.read_memory(0x1e200,4096,space='vram')
                # Timer may have advanced while the dialog was visible.
                timer=15*128+28*2
                assert before[:timer]==after[:timer] and before[timer+16:]==after[timer+16:]
                for index in editable:
                    r,c=divmod(index,cols)
                    move(variable('_offset_x')*16+c*32+16,variable('_offset_y')*16+r*32+16,True)
                    key(str(cells[index]&15))
                capture('game-completed')
                key('Return',180);ready()
                assert status(number)&1
            print('PASS: four-color 640x480 gameplay, five sizes, entry/erase/check, input bounds,')
            print('      quit/cancel restoration, completion and solved progress.')
            if args.screenshots:print('Screenshots:',captures)

if __name__=='__main__':main()
