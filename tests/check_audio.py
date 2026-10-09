"""Verify the game's actual audio glue against ZSM PSG writes in AgentBridge.

Each effect runs over music and must stop its own voices. Runtime copies keep
puzzle progress untouched. Only the emulator Python API is required.
"""
from runtime_assets import copy_runtime

import re
import shutil
import sys
import tempfile
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT.parent/'x16-emulator/api/python'))
from x16agent import X16Agent
PSG=0x1F9C0
symbols={n:int(a,16) for a,n in re.findall(r'al ([0-9A-Fa-f]+) \.(\S+)',(ROOT/'src/KAKURO.sym').read_text())}
offsets={n:int(a,16) for n,a in re.findall(r'#define (SFX_\w+)\s+(0x[0-9A-Fa-f]+)',(ROOT/'src/sfx.h').read_text()) if n!='SFX_BANK_ADDR'}


def decode(data):
    """Decode the PSG-only streams generated for Kakuro, including delays."""
    assert data[:3]==b'zm\x01' and data[9]==0
    mask=int.from_bytes(data[10:12],'little')
    loop=int.from_bytes(data[3:6],'little')
    pos=16;frames=[[]];loop_tick=None
    while True:
        if pos==loop:loop_tick=len(frames)-1
        cmd=data[pos];pos+=1
        if cmd<64:
            frames[-1].append((cmd,data[pos]));pos+=1
        elif cmd==128:break
        elif cmd>128:frames.extend([] for _ in range(cmd&127))
        else:raise AssertionError('Unexpected non-PSG command')
    if not frames[-1]:frames.pop()
    return mask,frames,loop_tick


def distinct(items):
    result=[]
    for item in items:
        if not result or item!=result[-1]:result.append(item)
    return result


with tempfile.TemporaryDirectory(prefix='kakuro-audio-') as temp:
    runtime=Path(temp)
    copy_runtime(ROOT/'src', runtime)
    with (runtime/'emulator.log').open('w') as log,X16Agent(
        ROOT.parent/'x16-emulator/build/x16emu',ROOT.parent/'emulator/rom.bin',fsroot=runtime,
        options=['-prg',str(runtime/'KAKURO.PRG'),'-run'],stderr=log) as emu:
        def advance(frames):assert emu.advance(frames)['reason']=='target'
        def key(name):
            emu.request('key',key=name,down=True);advance(8)
            emu.request('key',key=name,down=False);advance(24)
        def ready(entry):
            emu.request('breakpoint',address=symbols[entry])
            for _ in range(5):
                result=emu.advance(100000000,unit='cycles')
                if result['reason']=='breakpoint':break
            emu.request('breakpoint',action='clear');assert result['reason']=='breakpoint',result
        def invoke(entry,arg):
            # Call the linked fastcall assembly through a temporary JSR stub,
            # then resume the paused game with its CPU registers intact.
            state=emu.request('status')
            address=symbols[entry]
            emu.write_memory(0x400,bytes((0x20,address&255,address>>8,0x4c,3,4)))
            emu.request('set_registers',pc=0x400,a=arg&255,x=arg>>8)
            emu.request('breakpoint',address=0x403)
            result=emu.advance(1000000,unit='cycles');assert result['reason']=='breakpoint',result
            emu.request('breakpoint',action='clear')
            emu.request('set_registers',**{n:state[n] for n in ('pc','a','x','y','sp','p','ram_bank','rom_bank')})
        def expect_key_sound(name,effect):
            emu.request('breakpoint',address=symbols['_play_sfx'])
            emu.request('key',key=name,down=True)
            result=emu.advance(1000000,unit='cycles')
            assert result['reason']=='breakpoint',(name,result)
            state=result['state'];assert state['a']+(state['x']<<8)==offsets[effect],(name,effect,state)
            emu.request('breakpoint',action='clear');advance(8)
            emu.request('key',key=name,down=False);advance(24)
        def move(x,y):
            for _ in range(8):
                pos=emu.read_memory(2,4);px=int.from_bytes(pos[:2],'little');py=int.from_bytes(pos[2:],'little')
                if (px,py)==(x,y):return
                emu.request('mouse',dx=max(-255,min(255,x-px)),dy=max(-255,min(255,y-py)));advance(12)
            raise AssertionError('Mouse did not reach cell')
        def verify(data,entry,arg,label,music=False):
            mask,frames,loop=decode(data)
            regs=[r for r in range(64) if mask>>(r//4)&1]
            before=emu.read_memory(PSG,64,space='vram')
            invoke(entry,arg)
            count=180 if music else len(frames)+30
            snaps=[]
            for _ in range(count):
                advance(1);snaps.append(emu.read_memory(PSG,64,space='vram'))
            state=list(before);reference=[tuple(state[r] for r in regs)];index=0
            for _ in range(count+120):
                if index>=len(frames):
                    if loop is None:break
                    index=loop
                for r,v in frames[index]:state[r]=v
                reference.append(tuple(state[r] for r in regs));index+=1
            expected=distinct(reference);actual=distinct([tuple(s[r] for r in regs) for s in snaps])
            cursor=0;transients=0
            for item in actual:
                try:cursor=expected.index(item,cursor)
                except ValueError:transients+=1
            assert transients<=2 if music else transients==0,(label,transients,len(actual))
            assert len(actual)>2,(label,'No audible register changes')
            if not music:assert all(snaps[-1][r]&63==0 for r in regs if r%4==2),label
            print(f'PASS {label}: {len(actual)} PSG states match the asset',flush=True)
        advance(300)
        assert emu.read_memory(0x1000,3110,space='banked_ram',bank=4)==(ROOT/'src/SFX.BIN').read_bytes()
        # The start screen already plays the menu track.
        assert emu.read_memory(symbols['_sound_current_scene'])[0]==0
        assert any(emu.read_memory(PSG+v*4+2,space='vram')[0]&63 for v in range(10))
        key('Return');ready('_menu_handle_mouse')
        for name in ('MENU','GAME'):
            emu.write_memory(0x410,name.lower().encode()+b'.zsm\0')
            data=(ROOT/f'src/{name}.ZSM').read_bytes()
            verify(data,'_play_music',0x410,name,music=True)
            _,frames,_=decode(data)
            remaining=len(frames)+60-180
            while remaining>0:
                step=min(remaining,300);advance(step);remaining-=step
            states=[]
            for _ in range(90):
                advance(1);states.append(emu.read_memory(PSG,40,space='vram'))
            assert len(distinct(states))>4,(name,'Music failed to loop')
            assert any(states[-1][v*4+2]&63 for v in range(10)),name
            print(f'PASS {name}: continues past its loop boundary',flush=True)
        blob=(ROOT/'src/SFX.BIN').read_bytes()
        for name,offset in offsets.items():verify(blob[offset:],'_play_sfx',offset,name)
        # Turning off music silences only music voices; effects still play.
        invoke('_stop_bgmusic',0);emu.write_memory(symbols['_music'],b'\0');advance(12)
        assert all(emu.read_memory(PSG+v*4+2,space='vram')[0]&63==0 for v in range(10))
        verify(blob[offsets['SFX_SELECT']:],'_play_sfx',offsets['SFX_SELECT'],'effect with Music OFF')
        invoke('_start_bgmusic',0);emu.write_memory(symbols['_music'],b'\1');advance(60)
        assert any(emu.read_memory(PSG+v*4+2,space='vram')[0]&63 for v in range(10))
        # Actual gameplay and return paths must select the right track.
        expect_key_sound('Return','SFX_SELECT');ready('_puzzle_handle_mouse')
        assert emu.read_memory(symbols['_sound_current_scene'])[0]==1
        rows=emu.read_memory(symbols['_puzzlerows'])[0];cols=emu.read_memory(symbols['_puzzlecols'])[0]
        pointer=int.from_bytes(emu.read_memory(symbols['_puzzledata'],2),'little')
        cells=emu.read_memory(pointer,rows*cols)
        index=next(i for i,c in enumerate(cells) if not c&0x30)
        row,col=divmod(index,cols)
        move(emu.read_memory(symbols['_offset_x'])[0]*16+col*32+16,
             emu.read_memory(symbols['_offset_y'])[0]*16+row*32+16)
        for digit in range(1,10):expect_key_sound(str(digit),f'SFX_PLACE{digit}')
        expect_key_sound('Backspace','SFX_BACK')
        expect_key_sound('Escape','SFX_DIALOG');expect_key_sound('N','SFX_BACK');ready('_puzzle_handle_mouse')
        key('Escape');expect_key_sound('Y','SFX_BACK');ready('_menu_handle_mouse')
        assert emu.read_memory(symbols['_sound_current_scene'])[0]==0
        assert emu.read_memory(0x1000,3110,space='banked_ram',bank=4)==blob
        print('PASS: title music, both tracks, all 18 effects, mute/re-enable, scene switching and effect bank survival.')
