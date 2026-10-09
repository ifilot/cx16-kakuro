"""Encode the journal background and an opaque four-color mouse cursor."""

from pathlib import Path
import sys

import numpy as np
from PIL import Image, ImageDraw

from create_puzzle import read_file
from difficulty import blossom


ASSETS = Path(__file__).resolve().parent.parent
COLORS = [(204, 204, 153), (136, 102, 102), (68, 51, 51), (34, 34, 34)]


def pack(image):
    pixels = np.asarray(image.convert('RGB'))
    colors = np.array(COLORS, dtype=np.uint8)
    indices = np.zeros(pixels.shape[:2], dtype=np.uint8)
    for index, color in enumerate(colors):
        indices[np.all(pixels == color, axis=2)] = index
    if not np.array_equal(colors[indices], pixels):
        raise ValueError('Menu artwork must use exactly the four journal colors')
    return ((indices[:, 0::4] << 6) | (indices[:, 1::4] << 4)
            | (indices[:, 2::4] << 2) | indices[:, 3::4]).tobytes()


def glyphs(image, font, s, x, y, color, scale=1):
    for char in s:
        index = ord(char) - 32
        gx, gy = index % 16 * 8, index // 16 * 8
        mask = font.crop((gx, gy, gx + 8, gy + 8)).getchannel('A')
        mask = mask.point(lambda a: 255 if a > 150 else 0)
        mask = mask.resize((8 * scale, 8 * scale), Image.Resampling.NEAREST)
        image.paste(color, (x, y), mask)
        x += 8 * scale


def panel(image, box, face, border):
    x, y, width, height = box
    draw = ImageDraw.Draw(image)
    points = [(x+2,y), (x+width-3,y), (x+width-1,y+2),
              (x+width-1,y+height-3), (x+width-3,y+height-1),
              (x+2,y+height-1), (x,y+height-3), (x,y+2)]
    draw.polygon(points, fill=face, outline=border)


def card(font, number, shape, difficulty, state):
    paper, rose, brown, ink = COLORS
    image = Image.new('RGB', (64, 56), paper)
    face = brown if state == 1 else rose if state == 2 else paper
    label = brown if face == paper else paper
    panel(image, (7, 7, 56, 48), ink, ink)
    panel(image, (4, 4, 56, 48), face, brown)
    if state == 1:
        panel(image, (2, 2, 60, 52), brown, brown)
        panel(image, (4, 4, 56, 48), face, paper)
        ImageDraw.Draw(image).line((9, 7, 54, 7), fill=rose)
    glyphs(image, font, f'{number:02}', 16, 12, label, 2)
    size = f'{shape[0]}x{shape[1]}'
    glyphs(image, font, size, 11, 38, label)
    draw = ImageDraw.Draw(image)
    if len(size) <= 3:
        for flower in range(difficulty):
            blossom(image,39+flower*5,40,True,COLORS,mini=True,face=label)
    if state == 2:
        panel(image, (49, 8, 8, 8), face, label)
    elif state == 3:
        panel(image, (49, 8, 8, 8), brown, brown)
        draw.line((50,11,52,13,55,10), fill=paper)
    return pack(image).ljust(1024, b'\x00')


def button_image(font, label, highlight, background=None):
    paper, rose, brown, ink = COLORS
    width = len(label)*16+20
    image = background.copy() if background is not None else Image.new(
        'RGB', (184 if label.startswith('MUSIC:') else width+4, 36), paper)
    panel(image, (2,3,width,32), ink, ink)
    panel(image, (0,0,width,32), brown if highlight else paper, brown)
    ImageDraw.Draw(image).line((4,3,width-5,3), fill=rose if highlight else paper)
    glyphs(image,font,label,10,8,paper if highlight else brown,2)
    return pack(image).ljust(2048, b'\x00')


def details_image(font, number, shape, difficulty, state):
    paper, rose, brown, ink = COLORS
    image = Image.new('RGB', (448,21), paper)
    ImageDraw.Draw(image).rectangle((0,0,447,20),outline=brown)
    status = ['READY TO PLAY','OPENED','SOLVED'][state]
    label = f'PUZZLE {number:02}  {shape[0]}x{shape[1]}  {status}'
    glyphs(image,font,label,12,6,brown)
    for flower in range(5):
        blossom(image,350+flower*17,4,flower<difficulty,COLORS)
    return pack(image).ljust(4096,b'\x00')


def main():
    output = Path(sys.argv[1]) if len(sys.argv) > 1 else Path('.')
    output.mkdir(parents=True, exist_ok=True)
    palette = Image.new('P', (1, 1))
    palette.putpalette([value for color in COLORS for value in color] * 64)
    with Image.open(ASSETS / 'menu/concepts/03-puzzle-journal-source.png') as source:
        image = source.convert('RGB').resize((640, 480), Image.Resampling.LANCZOS)
        indices = np.asarray(image.quantize(palette=palette, dither=Image.Dither.FLOYDSTEINBERG)) % 4
    image = Image.fromarray(np.array(COLORS, dtype=np.uint8)[indices])
    font = Image.open(ASSETS / 'tiles/font-tiles-8.png').convert('RGBA')
    paper, rose, brown, ink = COLORS
    draw = ImageDraw.Draw(image)
    draw.rectangle((176,24,463,79), fill=paper, outline=brown)
    glyphs(image, font, 'KAKURO', 248, 32, brown, 3)
    glyphs(image, font, 'SELECT A PUZZLE', 260, 63, brown)
    draw.rectangle((96,344,543,364), fill=paper, outline=brown)
    draw.rectangle((164,372,475,391), fill=paper)
    prefix = 'o OPENED    v SOLVED    '
    left = (640-(len(prefix)+len('DIFFICULTY'))*8-17)//2
    glyphs(image,font,prefix,left,378,brown)
    blossom(image,left+len(prefix)*8,376,True,COLORS)
    glyphs(image,font,'DIFFICULTY',left+len(prefix)*8+17,378,brown)
    for x, label in [(64,'HELP'), (160,'OPTIONS'), (304,'ABOUT')]:
        width = len(label)*16+20
        panel(image, (x+2,419,width,32), ink, ink)
        panel(image, (x,416,width,32), paper, brown)
        glyphs(image, font, label, x+10,424,brown,2)
    panel(image, (426,419,160,32), ink, ink)
    panel(image, (424,416,160,32), paper, brown)
    packed = pack(image)
    (output / 'JOURNAL0.DAT').write_bytes(packed[:0xF000])
    (output / 'JOURNAL1.DAT').write_bytes(packed[0xF000:])
    # Sprite index 0 is transparent, so index 4 duplicates the paper color.
    colors = COLORS + [COLORS[0]]
    (output / 'JPALET.DAT').write_bytes(bytes(value for r, g, b in colors
        for value in (((g >> 4) << 4) | (b >> 4), r >> 4)))
    cursor = Image.new('L', (16, 16), 0)
    ImageDraw.Draw(cursor).polygon([(0, 0), (0, 12), (3, 9), (6, 14), (8, 13), (5, 8), (10, 8)], fill=4, outline=3)
    (output / 'JCURSOR.DAT').write_bytes(cursor.tobytes())
    controls = bytearray()
    footer_positions = {'HELP':64, 'OPTIONS':160, 'ABOUT':304}
    for label in ['HELP','OPTIONS','ABOUT','MUSIC: ON','MUSIC: OFF','BACK']:
        background = None
        if label in footer_positions:
            x = footer_positions[label]
            background = image.crop((x,416,x+len(label)*16+24,452))
        for highlight in range(2):
            controls.extend(button_image(font,label,highlight,background))
    # Selected-card progress badges include the nearby paper border.
    for solved in (False,True):
        badge = Image.new('RGB',(12,12),brown)
        d = ImageDraw.Draw(badge)
        d.line((11,0,11,11),fill=paper)
        panel(badge,(1,2,8,8),brown,brown if solved else paper)
        if solved: d.line((2,5,4,7,7,4),fill=paper)
        controls.extend(pack(badge).ljust(2048,b'\x00'))
    # Reserve two slots so pagination starts at bank 11.
    controls.extend(bytes(2*2048))
    for page in range(4):
        for hover in range(3):
            # Include both shadow columns and retain the courtyard around it.
            pagination = image.crop((424,416,588,452))
            panel(pagination,(2,3,160,32),ink,ink)
            panel(pagination,(0,0,160,32),paper,brown)
            d = ImageDraw.Draw(pagination)
            # Inset the active tab so it cannot merge into the outer border
            # or the stepped corners. Match the other buttons' rose accent.
            if (hover == 1 and page) or (hover == 2 and page < 3):
                left = 2 if hover == 1 else 130
                panel(pagination,(left,4,28,24),brown,brown)
                d.line((left+4,6,left+23,6),fill=rose)
            glyphs(pagination,font,'<',8,8,paper if hover == 1 and page else brown if page else rose,2)
            glyphs(pagination,font,'>',136,8,paper if hover == 2 and page < 3 else brown if page < 3 else rose,2)
            glyphs(pagination,font,f'PAGE {page+1}/4',40,12,brown)
            controls.extend(pack(pagination).ljust(2048,b'\x00'))
    (output / 'JUI.DAT').write_bytes(controls)
    (output / 'JRESTORE.DAT').write_bytes(controls[:16384])
    dialog = Image.new('RGB', (288,144), ink)
    d = ImageDraw.Draw(dialog)
    d.rectangle((2,2,285,141),fill=paper)
    d.rectangle((6,6,281,137),outline=brown)
    glyphs(dialog,font,'OPTIONS',88,16,brown,2)
    packed_dialog = pack(dialog)
    (output / 'JDIALOG.DAT').write_bytes(packed_dialog[:8064].ljust(8192,b'\x00') + packed_dialog[8064:])
    for page in range(4):
        details = bytearray()
        for state in range(3):
            for slot in range(24):
                number = page*24+slot+1
                shape, _, _, difficulty = read_file(ASSETS / f'puzzles/{number:03}.puz')
                details.extend(details_image(font,number,shape,difficulty,state))
        for part in range(6):
            (output / f'JDETAIL{page+1}{part}.DAT').write_bytes(details[part*49152:(part+1)*49152])
        templates = bytearray()
        for state in range(4):
            for slot in range(24):
                number = page*24+slot+1
                shape, _, _, difficulty = read_file(ASSETS / f'puzzles/{number:03}.puz')
                templates.extend(card(font, number, shape, difficulty, state))
        (output / f'JPAGE{page+1}0.DAT').write_bytes(templates[:49152])
        (output / f'JPAGE{page+1}1.DAT').write_bytes(templates[49152:])
    print('Journal: 640x480, 4 colors, 76800 bitmap bytes')


if __name__ == '__main__':
    main()
