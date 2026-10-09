"""Measure scene latency and LOAD calls, checking that loading keeps L0 visible.

Measurements include input delivery, rendering, audio and the paper fade at
8 MHz. Temporary runtime copies protect saved puzzle status.
"""
import re,sys,shutil,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT.parent/'x16-emulator/api/python'))
from x16agent import X16Agent
sym={n:int(a,16) for a,n in re.findall(r'al ([\da-fA-F]+) \.(\S+)',(ROOT/'src/KAKURO.sym').read_text())}
with tempfile.TemporaryDirectory() as temp:
 p=Path(temp)
 for pattern in ('*.DAT','*.ZSM','*.BIN','KAKURO.PRG'):
  for f in (ROOT/'src').glob(pattern):shutil.copy2(f,p/f.name)
 with X16Agent(ROOT.parent/'x16-emulator/build/x16emu',ROOT.parent/'emulator/rom.bin',fsroot=p,options=['-prg',str(p/'KAKURO.PRG'),'-run']) as e:
  def transition(label,event,entry):
   e.request('breakpoint',address=sym[entry]);e.request('breakpoint',address=0xFFD5);start=e.request('status')['cycles'];event();loads=0
   for _ in range(1000):
    assert e.read_memory(0x9F29)[0]&0x10,(label,'Display blanked')
    r=e.advance(1)
    if r['reason']=='breakpoint':
     if r['state']['pc']==sym[entry]:break
     assert r['state']['pc']==0xFFD5,r
     loads+=1
     e.request('breakpoint',action='clear');e.advance(1,unit='instructions')
     e.request('breakpoint',address=sym[entry]);e.request('breakpoint',address=0xFFD5)
   e.request('breakpoint',action='clear');assert r['reason']=='breakpoint',r
   cost=e.request('status')['cycles']-start;print(label,cost,round(cost/8000,2),'ms;',loads,'KERNAL LOAD breakpoint hits',flush=True)
  def key(name):e.request('key',key=name,down=True)
  def release(name):e.request('key',key=name,down=False);e.advance(12)
  e.advance(300)
  transition('Title -> Journal',lambda:key('Return'),'_menu_handle_mouse');release('Return')
  pos=e.read_memory(2,4);x=int.from_bytes(pos[:2],'little');y=int.from_bytes(pos[2:],'little')
  while(x,y)!=(106,432):
   dx=max(-255,min(255,106-x));dy=max(-255,min(255,432-y));e.request('mouse',dx=dx,dy=dy);e.advance(12);x+=dx;y+=dy
  e.request('mouse',buttons=1);e.advance(12)
  transition('Journal -> Help',lambda:e.request('mouse',buttons=0),'_docview_handle_key')
  transition('Help -> Journal',lambda:key('Escape'),'_menu_handle_mouse');release('Escape')
  transition('Journal -> Game',lambda:key('Return'),'_puzzle_handle_mouse');release('Return')
  # Escape and the Keep Playing button both cancel the new modal.
  before=e.read_memory(0x1e200,4096,space='vram')
  key('Escape');e.advance(12);release('Escape')
  key('Escape');e.advance(12);release('Escape')
  after=e.read_memory(0x1e200,4096,space='vram')
  timer=15*128+28*2
  assert before[:timer]==after[:timer] and before[timer+16:]==after[timer+16:]
  key('Escape');e.advance(12);release('Escape')
  pos=e.read_memory(2,4);x=int.from_bytes(pos[:2],'little');y=int.from_bytes(pos[2:],'little')
  while (x,y)!=(408,266):
   dx=max(-255,min(255,408-x));dy=max(-255,min(255,266-y));e.request('mouse',dx=dx,dy=dy);e.advance(12);x+=dx;y+=dy
  e.request('mouse',buttons=1);e.advance(8);e.request('mouse',buttons=0);e.advance(12)
  assert not e.read_memory(sym['_gamestate'])[0]&2
  key('Escape');e.advance(12);release('Escape')
  transition('Game -> Journal',lambda:key('Y'),'_menu_handle_mouse');release('Y')

print('PASS: scene switches keep a visible layer; exit modal keyboard/mouse cancellation works.')
