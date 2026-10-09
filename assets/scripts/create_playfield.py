"""Build the four-color, 640x480 journal playfield and 4bpp hardware tiles."""
from pathlib import Path
import sys
import json
import numpy as np
from PIL import Image, ImageDraw
from create_menu import ASSETS, COLORS, pack, glyphs, panel


def tiles(image):
    colors = np.array(COLORS,dtype=np.uint8)
    pixels = np.asarray(image.convert('RGB'))
    indices = np.zeros(pixels.shape[:2],dtype=np.uint8)
    # Zero is transparent in tile mode; index 4 is opaque paper.
    for index,color in enumerate(colors):
        indices[np.all(pixels == color,axis=2)] = (4 if index == 0 else index)
    assert np.array_equal(colors[indices % 4],pixels)
    return b''.join(((indices[y:y+16,x:x+16:2]<<4)|indices[y:y+16,x+1:x+16:2]).tobytes()
                    for y in range(0,image.height,16) for x in range(0,image.width,16))


def main():
    output = Path(sys.argv[1]) if len(sys.argv)>1 else Path('.')
    output.mkdir(parents=True,exist_ok=True)
    font = Image.open(ASSETS/'tiles/ui-charmap.png').convert('RGBA')
    small = Image.open(ASSETS/'tiles/clue-charmap.png').convert('RGBA')
    paper,rose,brown,ink = COLORS
    # Start from the same quantized artwork, before the menu's UI panels.
    palette=Image.new('P',(1,1));palette.putpalette([v for c in COLORS for v in c]*64)
    with Image.open(ASSETS/'backgrounds/journal.png') as source:
        indices=np.asarray(source.convert('RGB').resize((640,480),Image.Resampling.LANCZOS).quantize(palette=palette,dither=Image.Dither.FLOYDSTEINBERG))%4
    background=Image.fromarray(np.array(COLORS,dtype=np.uint8)[indices])
    for box in ((50,27,560,48),(50,83,352,352)):
        panel(background,box,ink,ink)
    for box in ((48,24,560,48),(48,80,352,352)):
        panel(background,box,paper,brown)
    panel(background,(432,80,176,352),paper,paper)
    ImageDraw.Draw(background).line((432,100,432,411),fill=rose)
    glyphs(background,font,'KAKURO',72,32,brown,3)
    glyphs(background,font,'SAKURA JOURNAL',464,43,brown)
    glyphs(background,font,'PUZZLE',448,84,rose)
    glyphs(background,font,'DIFFICULTY',448,164,rose)
    glyphs(background,font,'ELAPSED TIME',448,220,rose)
    panel(background,(48,434,560,28),brown,rose)
    glyphs(background,font,'MOVE MOUSE   1-9 WRITE   DEL ERASE',64,440,paper)
    glyphs(background,font,'ESC TO RETURN',448,394,rose)
    packed=pack(background)
    (output/'GPLAY0.DAT').write_bytes(packed[:61440])
    (output/'GPLAY1.DAT').write_bytes(packed[61440:])
    gfx=bytearray(128) # tile 0 is transparent
    layout=json.loads((ASSETS/'tiles/layout.json').read_text())
    source=Image.open(ASSETS/'tiles/cells.png').convert('RGB')
    columns,rows=layout['cells_grid']
    assert layout['tile_size']==[16,16] and layout['cell_size']==[32,32]
    assert len(layout['cells'])==49 and source.size==(columns*32,rows*32)
    for kind in range(len(layout['cells'])):
        x,y=kind%columns*32,kind//columns*32
        gfx.extend(tiles(source.crop((x,y,x+32,y+32))))
    cell_first=1
    clue_first=len(gfx)//128
    clue_kind=layout['cell_kinds']['clue']
    x,y=clue_kind%columns*32,clue_kind//columns*32
    clue=source.crop((x,y,x+32,y+32))
    for value in range(2,46):
        for direction in range(2):
            image=clue.crop((16,0,32,16) if direction==0 else (0,16,16,32))
            s=str(value)
            digits=Image.new('L',(len(s)*5-1,8),0)
            x=0
            for char in s:
                mask=small.crop((int(char)*4,0,int(char)*4+4,8)).getchannel('R').point(lambda v: 255 if v>150 else 0)
                digits.paste(mask,(x,0));x+=5
            digits=digits.crop(digits.getbbox())
            image.paste(paper,((16-digits.width)//2,(16-digits.height)//2),digits)
            gfx.extend(tiles(image))
    font_first=len(gfx)//128
    for code in range(32,91):
        image=Image.new('RGB',(16,16),paper)
        glyphs(image,font,chr(code),0,0,brown,2)
        gfx.extend(tiles(image))
    border_first=len(gfx)//128
    borders=Image.open(ASSETS/'tiles/dialog-borders.png').convert('RGB')
    assert borders.size==(80,16)
    gfx.extend(tiles(borders))
    difficulty_first=len(gfx)//128
    flowers=Image.open(ASSETS/'tiles/difficulty.png').convert('RGBA')
    assert flowers.size==(48,16)
    opaque=Image.new('RGBA',(32,16),(*paper,255))
    opaque.alpha_composite(flowers.crop((0,0,32,16)))
    gfx.extend(tiles(opaque))
    assert 0x13000+len(gfx)<=0x1e000
    definitions={'GTILE_CELL_FIRST':cell_first,'GTILE_CLUE_FIRST':clue_first,
        'GTILE_FONT_FIRST':font_first,'GTILE_BORDER_FIRST':border_first,
        'GTILE_DIFFICULTY_FILLED':difficulty_first,'GTILE_DIFFICULTY_OUTLINE':difficulty_first+1}
    definitions.update({'CELL_'+key.upper():value for key,value in layout['cell_kinds'].items()})
    header='/* Generated from assets/tiles/layout.json and the live source sheets. */\n#ifndef TILE_LAYOUT_H\n#define TILE_LAYOUT_H\n'
    header+=''.join(f'#define {key} {value}\n' for key,value in definitions.items())+'#endif\n'
    (output/'tile_layout.h').write_text(header)
    # The exit modal is a cached bitmap, using the same font and paper panels.
    modal=Image.new('RGB',(384,96),brown)
    panel(modal,(3,3,381,93),ink,ink)
    panel(modal,(0,0,381,93),paper,brown)
    ImageDraw.Draw(modal).line((12,12,12,80),fill=rose)
    glyphs(modal,font,'LEAVE THIS PUZZLE?',48,16,brown,2)
    glyphs(modal,font,'Unfinished entries will be cleared.',56,42,rose)
    buttons=bytearray()
    for label,x in (('Y  LEAVE',40),('N  KEEP PLAYING',216)):
        for state in range(2):
            button=Image.new('RGB',(128,20),paper)
            panel(button,(0,0,128,20),brown if state else paper,rose)
            glyphs(button,font,label,(128-len(label)*8)//2,6,paper if state else brown)
            buttons.extend(pack(button))
            if not state:modal.paste(button,(x,64))
    (output/'GQUIT.DAT').write_bytes(pack(modal)+buttons)
    (output/'GTILES.DAT').write_bytes(gfx)
    # Controls are cached in RAM banks 7-8 and blitted to the bitmap.
    controls=bytearray()
    for label in ('CHECK OFF','CHECK ON'):
        for hover in range(2):
            image=background.crop((436,288,604,324))
            d=ImageDraw.Draw(image)
            d.rectangle((12,12,21,21),outline=rose)
            if label=='CHECK ON': d.line((14,16,16,18,19,14),fill=brown)
            glyphs(image,font,'CHECK: '+('ON' if label=='CHECK ON' else 'OFF'),32,14,brown)
            if hover:
                d.line((12,27,151,27),fill=rose)
            else:
                d.line((12,27,151,27),fill=paper)
            controls.extend(pack(image).ljust(2048,b'\x00'))
    (output/'GCONTROLS.DAT').write_bytes(controls)
    print(f'Playfield: 640x480, four colors, {len(gfx)} bytes of 4bpp tiles')

if __name__=='__main__':main()
