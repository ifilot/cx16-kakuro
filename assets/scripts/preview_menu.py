"""Build five 640x480 four-color menu mockups with the game's bitmap font."""

from pathlib import Path
import json
import re

import numpy as np
from PIL import Image, ImageDraw


ASSETS = Path(__file__).resolve().parent.parent
PALETTE = [(204, 204, 153), (136, 102, 102), (68, 51, 51), (34, 34, 34)]
NAMES = ['01-courtyard-board', '02-shoji-paper', '03-puzzle-journal',
         '04-hanging-scroll', '05-evening-veranda']


def main():
    root = ASSETS / 'menu/concepts'
    font = Image.open(ASSETS / 'tiles/font-tiles-8.png').convert('RGBA')
    fixed = Image.new('P', (1, 1))
    fixed.putpalette([v for color in PALETTE for v in color] * 64)
    report = []
    for name in NAMES:
        with Image.open(root / (name + '-source.png')) as source:
            scene = source.convert('RGB').resize((640, 480), Image.Resampling.LANCZOS)
            indexed = scene.quantize(palette=fixed, dither=Image.Dither.FLOYDSTEINBERG)
            image = indexed.convert('RGB')
        dark = name.startswith('05')
        paper, rose, brown, ink = PALETTE
        foreground, background = (paper, brown) if dark else (brown, paper)
        draw = ImageDraw.Draw(image)

        def text(s, x, y, color=foreground, scale=1):
            for char in s:
                code = ord(char) - 32
                if not 0 <= code < 96:
                    raise ValueError(char)
                gx, gy = (code % 16) * 8, (code // 16) * 8
                mask = font.crop((gx, gy, gx + 8, gy + 8)).getchannel('A')
                mask = mask.point(lambda a: 255 if a > 150 else 0)
                if scale != 1:
                    mask = mask.resize((8 * scale, 8 * scale), Image.Resampling.NEAREST)
                image.paste(color, (x, y), mask)
                x += 8 * scale

        def centered(s, y, color=foreground, scale=1):
            text(s, (640 - len(s) * 8 * scale) // 2, y, color, scale)

        # Quiet title strip and real bitmap glyphs: no generated lettering.
        draw.rectangle((176, 24, 463, 79), fill=background, outline=foreground)
        centered('KAKURO', 32, scale=3)
        centered('SELECT A PUZZLE', 63)

        # Same 48-item information hierarchy across all visual directions.
        for row in range(6):
            for col in range(8):
                number = row * 8 + col + 1
                x = 108 + col * 52 + (12 if col >= 4 else 0)
                y = 106 + row * 36
                selected = number == 1
                face, label = (foreground, background) if selected else (background, foreground)
                if name.startswith('02'):
                    draw.rectangle((x, y, x + 39, y + 31), fill=face)
                    draw.line((x, y + 31, x + 39, y + 31), fill=label)
                else:
                    draw.rectangle((x + 2, y + 2, x + 41, y + 33), fill=ink)
                    draw.rectangle((x, y, x + 39, y + 31), fill=face, outline=label)
                if selected:
                    draw.rectangle((x - 2, y - 2, x + 41, y + 33), outline=foreground)
                if name.startswith('04'):
                    draw.line((x + 3, y + 1, x + 3, y + 30), fill=rose)
                text(f'{number:02}', x + 5, y + 4, label, scale=2)
                puzzle = (ASSETS / f'puzzles/{number:03}.puz').read_text()
                lines = [line for line in puzzle.splitlines() if line.strip() and not line.startswith('#')]
                size = len(lines)
                difficulty = re.search(r'/(\d)\*', puzzle)
                stars = int(difficulty.group(1)) if difficulty else 1
                text(f'{size}x{size}', x + 3, y + 23, label)
                for star in range(min(stars, 3)):
                    draw.point((x + 29 + star * 3, y + 26), fill=label)
                    draw.point((x + 29 + star * 3, y + 25), fill=label)
                if number == 2:  # Illustrative opened state.
                    draw.ellipse((x + 32, y + 3, x + 36, y + 7), outline=label)
                if number == 3:  # Illustrative solved state.
                    draw.line((x + 31, y + 5, x + 33, y + 7, x + 37, y + 3), fill=label, width=1)

        draw.rectangle((96, 344, 543, 364), fill=background, outline=foreground)
        centered('PUZZLE 01   6x6   **   READY TO PLAY', 350)
        draw.rectangle((164, 372, 475, 391), fill=background)
        centered('o OPENED    v SOLVED    . DIFFICULTY', 378)
        for x, label in [(72, 'HELP'), (160, 'OPTIONS'), (272, 'ABOUT')]:
            width = len(label) * 8 + 20
            draw.rectangle((x, 417, x + width, 438), fill=background, outline=foreground)
            text(label, x + 10, 424)
        draw.rectangle((384, 417, 567, 438), fill=background, outline=foreground)
        text('<  PAGE 1/2  >', 420, 424)

        # Validate and encode the complete mockup as a real VERA 2bpp bitmap.
        pixels = np.asarray(image)
        colors = np.array(PALETTE, dtype=np.uint8)
        indices = np.zeros((480, 640), dtype=np.uint8)
        for index, color in enumerate(colors):
            indices[np.all(pixels == color, axis=2)] = index
        assert np.array_equal(colors[indices], pixels)
        packed = ((indices[:, 0::4] << 6) | (indices[:, 1::4] << 4)
                  | (indices[:, 2::4] << 2) | indices[:, 3::4])
        assert packed.size == 76800
        image.save(root / (name + '.png'))
        (root / (name + '.bin')).write_bytes(packed.tobytes())
        report.append({'name': name, 'size': [640, 480], 'colors': 4,
                       'bitmap_bytes': packed.size, 'puzzles': 48,
                       'progress_states': 'illustrative selected/opened/solved'})
        print(name, '640x480, 4 colors, 48 puzzles, 76800 bytes')
    (root / 'previews.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
