"""Encode the selected 640x480 four-color courtyard as a 2bpp bitmap.

The two raw files load directly into consecutive VRAM ranges using KERNAL
LOAD with secondary address 2. Each stays below the 64 KiB transfer boundary.
"""

from pathlib import Path
import sys

import numpy as np
from PIL import Image


ASSETS = Path(__file__).resolve().parent.parent
WIDTH, HEIGHT = 640, 480
SPLIT = 0xF000
PALETTE_INDICES = (65, 35, 33, 18)


def main():
    output = Path(sys.argv[1]) if len(sys.argv) > 1 else Path('.')
    source = ASSETS / 'splash/comparisons/courtyard-640x480-4colors.png'
    with Image.open(source) as image:
        if image.size != (WIDTH, HEIGHT):
            raise ValueError('The selected splash must be exactly 640x480')
        pixels = np.asarray(image.convert('RGB'), dtype=np.int32).reshape(-1, 3)
    with Image.open(ASSETS / 'palette/cx16palette.png') as image:
        palette = np.array([
            image.getpixel((x * 64 + 32, y * 64 + 32))[:3]
            for y in range(16) for x in range(16)
        ], dtype=np.int32)

    four = palette[list(PALETTE_INDICES)]
    distances = ((pixels[:, None, :] - four[None, :, :]) ** 2).sum(axis=2)
    indices = distances.argmin(axis=1).astype(np.uint8)
    if not np.array_equal(four[indices], pixels):
        raise ValueError('The selected splash contains colors outside its four-color palette')
    indices = indices.reshape(HEIGHT, WIDTH)
    packed = ((indices[:, 0::4] << 6) | (indices[:, 1::4] << 4)
              | (indices[:, 2::4] << 2) | indices[:, 3::4])

    data = packed.tobytes()
    output.mkdir(parents=True, exist_ok=True)
    (output / 'SPLASH0.DAT').write_bytes(data[:SPLIT])
    (output / 'SPLASH1.DAT').write_bytes(data[SPLIT:])
    vera_palette = bytes(component for r, g, b in four
                         for component in (((int(g) >> 4) << 4) | (int(b) >> 4), int(r) >> 4))
    (output / 'SPLASHP.DAT').write_bytes(vera_palette)
    print(f'Splash: {WIDTH}x{HEIGHT}, 4 colors, {len(data)} bitmap bytes')


if __name__ == '__main__':
    main()
