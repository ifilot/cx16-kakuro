"""Render exact CX16 bitmap alternatives for the selected courtyard art."""

from pathlib import Path
import json

import numpy as np
from PIL import Image

from create_splash import ASSETS


def nearest_colors(pixels, palette):
    flat = np.asarray(pixels, dtype=np.int32).reshape(-1, 3)
    palette = np.asarray(palette, dtype=np.int32)
    indices = np.empty(len(flat), dtype=np.uint8)
    for start in range(0, len(flat), 1024):
        block = flat[start:start + 1024]
        distances = ((block[:, None, :] - palette[None, :, :]) ** 2).sum(axis=2)
        indices[start:start + len(block)] = distances.argmin(axis=1)
    return indices.reshape(pixels.height, pixels.width)


def main():
    output = ASSETS / 'splash/comparisons'
    output.mkdir(parents=True, exist_ok=True)
    with Image.open(ASSETS / 'palette/cx16palette.png') as image:
        palette = np.array([
            image.getpixel((x * 64 + 32, y * 64 + 32))[:3]
            for y in range(16) for x in range(16)
        ], dtype=np.uint8)
    with Image.open(ASSETS / 'splash/concepts/04-sakura-courtyard.png') as image:
        source = image.convert('RGB')

    low = source.resize((320, 240), Image.Resampling.LANCZOS)
    low_indices = nearest_colors(low, palette[1:]) + 1
    low_preview = Image.fromarray(palette[low_indices])
    low_preview.save(output / 'courtyard-320x240.png')
    low_preview.resize((640, 480), Image.Resampling.NEAREST).save(
        output / 'courtyard-320x240-displayed.png'
    )

    high = source.resize((640, 480), Image.Resampling.LANCZOS)
    # Derive four representative colors, then select actual CX16 palette entries.
    representative = np.array(high.quantize(colors=4).getpalette()[:12]).reshape(4, 3)
    distances = ((representative[:, None, :] - palette[None, :, :].astype(np.int32)) ** 2).sum(axis=2)
    selection = distances.argmin(axis=1)
    four = palette[selection]
    assert len(set(map(tuple, four))) == 4
    fixed_palette = Image.new('P', (1, 1))
    fixed_palette.putpalette(four.flatten().tolist() * 64)
    # Dithering represents intermediate tones using only these four colors.
    indexed = high.quantize(palette=fixed_palette, dither=Image.Dither.FLOYDSTEINBERG)
    high_indices = np.array(indexed) % 4
    Image.fromarray(four[high_indices]).save(output / 'courtyard-640x480-4colors.png')
    packed = ((high_indices[:, 0::4] << 6) | (high_indices[:, 1::4] << 4)
              | (high_indices[:, 2::4] << 2) | high_indices[:, 3::4])
    (output / 'courtyard-640x480-2bpp.bin').write_bytes(packed.tobytes())
    vera_palette = bytes(component for r, g, b in four
                         for component in (((int(g) >> 4) << 4) | (int(b) >> 4), int(r) >> 4))
    (output / 'courtyard-4colors-palette.bin').write_bytes(vera_palette)
    info = {
        'low_resolution': {'size': [320, 240], 'colors_used': len(np.unique(low_indices)), 'bitmap_bytes': low_indices.size},
        'high_resolution': {'size': [640, 480], 'colors_used': 4, 'bitmap_bytes': packed.size,
                            'palette_indices': selection.tolist(),
                            'palette_colors': ['#%02x%02x%02x' % tuple(color) for color in four],
                            'dithering': 'Floyd-Steinberg'},
    }
    (output / 'comparison.json').write_text(json.dumps(info, indent=2) + '\n')
    print(json.dumps(info, indent=2))


if __name__ == '__main__':
    main()
