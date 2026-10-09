"""Generate journal reading backgrounds, cached controls and wrapped text."""
from pathlib import Path
import sys
import textwrap
import struct
import json
import numpy as np
from PIL import Image, ImageDraw
from create_menu import ASSETS,COLORS,pack,glyphs,panel


def main():
    output=Path(sys.argv[1]) if len(sys.argv)>1 else Path('.')
    paper,rose,brown,ink=COLORS
    font=Image.open(ASSETS/'tiles/font-tiles-8.png').convert('RGBA')
    palette=Image.new('P',(1,1));palette.putpalette([v for c in COLORS for v in c]*64)
    with Image.open(ASSETS/'menu/concepts/03-puzzle-journal-source.png') as source:
        indices=np.asarray(source.convert('RGB').resize((640,480),Image.Resampling.LANCZOS).quantize(palette=palette,dither=Image.Dither.FLOYDSTEINBERG))%4
    base=Image.fromarray(np.array(COLORS,dtype=np.uint8)[indices])
    for box in ((50,27,544,56),(50,99,544,320),(50,427,544,40)):panel(base,box,ink,ink)
    for box in ((48,24,544,56),(48,96,544,320),(48,424,544,40)):panel(base,box,paper,brown)
    glyphs(base,font,'ESC TO RETURN',64,440,rose)
    glyphs(base,font,'ARROWS / PGUP PGDN',192,440,rose)
    for name,title,subtitle in (('HELP','HOW TO PLAY','RULES, CONTROLS AND PUZZLES'),('ABOUT','ABOUT KAKURO','COMMANDER X16 / SAKURA JOURNAL')):
        image=base.copy()
        glyphs(image,font,title,72,38,brown,2)
        glyphs(image,font,subtitle,72,64,rose)
        data=pack(image)
        (output/f'D{name}0.DAT').write_bytes(data[:61440])
        (output/f'D{name}1.DAT').write_bytes(data[61440:])
        lines=[]
        text=(output/f'{name}.TXT').read_text()
        if name=='ABOUT':text=text.format(**json.loads((output/'BUILDINFO.json').read_text()))
        for line in text.splitlines():
            heading=line.startswith('# ')
            line=line[2:] if heading else line
            wrapped=textwrap.wrap(line,width=60,subsequent_indent='  ' if line.startswith('- ') else '',break_long_words=False,break_on_hyphens=False) or ['']
            for text in wrapped:
                assert len(text)<=60
                lines.append(text.encode('ascii').ljust(60,b' ')+bytes((1 if heading else 2,0,0,0)))
        assert len(lines)<=127,'Document exceeds its reserved 8 KiB bank'
        (output/f'D{name}T.DAT').write_bytes(struct.pack('<H',len(lines))+b''.join(lines))
        print(f'{name}: {len(lines)} wrapped lines')
    controls=bytearray()
    for label,width,x in (('^ UP',64,440),('DOWN v',64,520)):
        for state in range(3):
            image=base.crop((x,432,x+width,456))
            glyphs(image,font,label,(width-len(label)*8)//2,8,rose if state==2 else brown)
            if state==1:ImageDraw.Draw(image).line((8,20,width-9,20),fill=rose)
            controls.extend(pack(image).ljust(1024,b'\x00'))
    (output/'DCONTROL.DAT').write_bytes(controls)

if __name__=='__main__':main()
