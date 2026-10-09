"""Export inspectable PNG sheets from the game's compiled DAT assets.

Run `make tilesheets` or `python3 assets/scripts/export_tilesheets.py`.
Raw sheets preserve native pixels and transparency; labeled sheets use 3x
nearest-neighbor previews and tile IDs. Cached bitmap controls/cards are
exported separately from the VERA hardware tile atlas.
"""
import argparse
import json
from pathlib import Path
from math import ceil
import numpy as np
from PIL import Image, ImageDraw

ROOT=Path(__file__).resolve().parents[2]
SRC=ROOT/'src'
COLORS=[(204,204,153),(136,102,102),(68,51,51),(34,34,34)]
TILE_COLORS=[(0,0,0,0)]+[(*COLORS[i],255) for i in (1,2,3,0)]


def unpack(data,width,height,bpp):
    raw=np.frombuffer(data,dtype=np.uint8)
    shifts=range(8-bpp,-1,-bpp)
    indices=np.stack([(raw>>shift)&((1<<bpp)-1) for shift in shifts],axis=1).reshape(-1)
    return indices[:width*height].reshape(height,width)


def bitmap(data,width,height):
    return Image.fromarray(np.array(COLORS,dtype=np.uint8)[unpack(data,width,height,2)])


def atlas(images,columns=16):
    image=Image.new('RGBA',(columns*16,ceil(len(images)/columns)*16))
    for i,tile in enumerate(images):image.paste(tile,(i%columns*16,i//columns*16))
    return image


def labeled_tiles(images):
    sheet=Image.new('RGB',(16*64,ceil(len(images)/16)*76),'#e6e0d0')
    draw=ImageDraw.Draw(sheet)
    for i,tile in enumerate(images):
        x=i%16*64;y=i//16*76
        draw.text((x+4,y+2),str(i),fill='#443333')
        sheet.paste(tile.resize((48,48),Image.Resampling.NEAREST),(x+4,y+18),tile.resize((48,48),Image.Resampling.NEAREST))
    return sheet


def cell_image(tiles,kind):
    image=Image.new('RGBA',(32,32))
    for quadrant in range(4):image.paste(tiles[1+kind*4+quadrant],(quadrant%2*16,quadrant//2*16))
    return image


def cells_sheet(tiles):
    groups=[('Special cells',[0,1,2,12]),('Written digits',list(range(3,12))),
            ('Selected digits',list(range(13,22))),('Given digits',list(range(22,31))),
            ('Verified correct',list(range(31,40))),('Verified incorrect',list(range(40,49)))]
    image=Image.new('RGB',(760,len(groups)*100),'#e6e0d0');draw=ImageDraw.Draw(image)
    for row,(name,kinds) in enumerate(groups):
        draw.text((12,row*100+12),name,fill='#443333')
        for col,kind in enumerate(kinds):
            x=140+col*68;y=row*100+26
            tile=cell_image(tiles,kind).resize((64,64),Image.Resampling.NEAREST)
            image.paste(tile,(x,y),tile)
            draw.text((x,y-14),f'{1+kind*4}-{4+kind*4}',fill='#443333')
    return image


def controls_sheet(data,items):
    image=Image.new('RGB',(760,ceil(len(items)/3)*104),'#e6e0d0');draw=ImageDraw.Draw(image)
    for i,(label,slot,width,height) in enumerate(items):
        x=i%3*252+8;y=i//3*104+8
        draw.text((x,y),label,fill='#443333')
        source=bitmap(data[slot*2048:],width,height)
        image.paste(source,(x,y+22))
        draw.rectangle((x-1,y+21,x+width,y+22+height),outline='#886666')
    return image


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=ROOT/'build/tilesheets')
    args=parser.parse_args();output=args.output;output.mkdir(parents=True,exist_ok=True)
    raw=(SRC/'GTILES.DAT').read_bytes()
    assert len(raw)%128==0
    palette=np.array(TILE_COLORS,dtype=np.uint8)
    tiles=[]
    for start in range(0,len(raw),128):
        indices=unpack(raw[start:start+128],16,16,4)
        assert indices.max()<len(palette)
        tiles.append(Image.fromarray(palette[indices]))
    atlas(tiles).save(output/'game-tiles.png')
    labeled_tiles(tiles).save(output/'game-tiles-labeled.png')
    cells_sheet(tiles).save(output/'game-cells.png')
    atlas(tiles[197:285],8).save(output/'game-clues.png')
    atlas(tiles[285:344]).save(output/'game-font.png')
    atlas(tiles[344:349],5).save(output/'game-dialog-borders.png')
    atlas(tiles[349:351],2).save(output/'game-difficulty.png')
    bitmap((SRC/'GQUIT.DAT').read_bytes(),384,96).save(output/'game-exit-dialog.png')
    controls=(SRC/'GCONTROLS.DAT').read_bytes()
    controls_sheet(controls,[(f'{label}: {state}',slot,168,36)
        for slot,(label,state) in enumerate((('Check off','normal'),('Check off','hover'),('Check on','normal'),('Check on','hover')))]).save(output/'game-controls.png')
    controls=(SRC/'JUI.DAT').read_bytes()
    items=[]
    for label,width in (('Help',88),('Options',136),('About',104),('Music on',184),('Music off',184),('Back',88)):
        for state in ('normal','hover'):items.append((f'{label}: {state}',len(items),width,36))
    items.extend([('Opened badge',12,12,12),('Solved badge',13,12,12)])
    for page in range(4):
        for state in range(3):items.append((f'Page {page+1}: '+('normal','previous','next')[state],16+page*3+state,164,36))
    controls_sheet(controls,items).save(output/'menu-controls.png')
    for page in range(1,5):
        raw=(SRC/f'JPAGE{page}0.DAT').read_bytes()+(SRC/f'JPAGE{page}1.DAT').read_bytes()
        sheet=Image.new('RGB',(384,4*248),'#e6e0d0');draw=ImageDraw.Draw(sheet)
        for state,label in enumerate(('Unopened','Selected','Opened','Solved')):
            draw.text((4,state*248+4),label,fill='#443333')
            for card in range(24):
                start=(state*24+card)*1024
                sheet.paste(bitmap(raw[start:start+896],64,56),(card%6*64,state*248+24+card//6*56))
        sheet.save(output/f'journal-page-{page}.png')
    for label,prefix in (('game-background','GPLAY'),('menu-background','JOURNAL'),('start-screen','SPLASH'),('help-background','DHELP'),('about-background','DABOUT')):
        raw=(SRC/f'{prefix}0.DAT').read_bytes()+(SRC/f'{prefix}1.DAT').read_bytes()
        bitmap(raw,640,480).save(output/f'{label}.png')
    indices=np.frombuffer((SRC/'JCURSOR.DAT').read_bytes(),dtype=np.uint8).reshape(16,16)
    Image.fromarray(palette[indices]).save(output/'cursor.png')
    manifest={'source':'src/GTILES.DAT','format':'4bpp, 16x16 tiles','vram_base':'0x13000',
        'tile_count':len(tiles),'sheet_columns':16,'palette':TILE_COLORS,
        'ranges':{'transparent':[0,0],'cells':[1,196],'clue_quadrants':[197,284],
                  'font_ASCII_32_to_90':[285,343],'dialog_borders':[344,348],'difficulty_filled_outline':[349,350]},
        'cell_kinds':{'blocked':0,'clue':1,'empty':2,'digits':[3,11],'selected_empty':12,
                      'selected_digits':[13,21],'given_digits':[22,30],
                      'correct_digits':[31,39],'incorrect_digits':[40,48]},
        'cell_tile_formula':'first tile = 1 + kind * 4; quadrants: top-left, top-right, bottom-left, bottom-right',
        'selected_incorrect':'Uses selected digit tiles with palette bank 1: opaque paper becomes rose.',
        'clue_formula':'tile = 197 + (sum - 2) * 2 + direction; right=0, down=1',
        'cached_bitmaps':{'game-controls.png':'src/GCONTROLS.DAT, 2048-byte slots',
                          'menu-controls.png':'src/JUI.DAT, 2048-byte slots',
                          'journal-page-1.png through journal-page-4.png':'src/JPAGE*.DAT, 1024-byte slots'},
        'previews':'Raw tile PNGs preserve native pixels and alpha. Labeled/cell sheets use nearest-neighbor enlargement.'}
    (output/'index.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print(f'Exported {len(tiles)} hardware tiles and current menu/game bitmaps to {output}')

if __name__=='__main__':main()
