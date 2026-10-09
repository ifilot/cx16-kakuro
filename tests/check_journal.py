"""Exercise the live journal menu through the local emulator's AgentBridge.

Run after `make`: python3 tests/check_journal.py --screenshots /tmp/journal-check
The emulator and ROM default to the neighboring CX16 workspace directories.
"""

import argparse
from pathlib import Path
import re
import shutil
import sys
import tempfile

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--emulator', type=Path, default=ROOT.parent / 'x16-emulator/build/x16emu')
    parser.add_argument('--rom', type=Path, default=ROOT.parent / 'emulator/rom.bin')
    parser.add_argument('--screenshots', type=Path)
    args = parser.parse_args()
    sys.path.insert(0, str(args.emulator.resolve().parent.parent / 'api/python'))
    from x16agent import X16Agent
    symbols = {name: int(address, 16) for address, name in re.findall(
        r'al ([0-9A-Fa-f]+) \.(\S+)', (ROOT / 'src/KAKURO.sym').read_text())}
    with Image.open(ROOT / 'assets/palette/cx16palette.png') as image:
        colors = [image.getpixel((x*64+32, y*64+32)) for y in range(16) for x in range(16)]
    game_palette = bytes(value for r, g, b in colors for value in ((g//16)*16+b//16, r//16))

    with tempfile.TemporaryDirectory(prefix='kakuro-journal-') as temporary:
        runtime = Path(temporary)
        captures = args.screenshots.resolve() if args.screenshots else runtime / 'screenshots'
        captures.mkdir(parents=True, exist_ok=True)
        for pattern in ('*.DAT', '*.ZSM', '*.BIN', '*.TXT', 'KAKURO.PRG'):
            for source in (ROOT / 'src').glob(pattern):
                shutil.copy2(source, runtime / source.name)
        with (runtime / 'emulator.log').open('w') as log, X16Agent(
                args.emulator.resolve(), args.rom.resolve(), fsroot=runtime,
                options=['-prg', str(runtime / 'KAKURO.PRG'), '-run'], stderr=log) as emu:
            def variable(name):
                return emu.read_memory(symbols[name])[0]

            def advance(frames):
                result = emu.advance(frames)
                assert result['reason'] == 'target', result

            def key(name, settle=180):
                emu.request('key', key=name, down=True)
                advance(8)
                emu.request('key', key=name, down=False)
                advance(settle)

            def ready():
                emu.request('breakpoint', address=symbols['_menu_handle_mouse'])
                for _ in range(3):
                    result = emu.advance(100000000, unit='cycles')
                    if result['reason'] == 'breakpoint':
                        break
                emu.request('breakpoint', action='clear')
                assert result['reason'] == 'breakpoint', result
                registers = emu.read_memory(0x9f2d, 3)
                assert registers[0] == 5 and registers[2] == 1
                assert emu.read_memory(0x1fa00, 10, space='vram') == (ROOT / 'src/JPALET.DAT').read_bytes()

            def move(x, y):
                for _ in range(8):
                    position = emu.read_memory(2, 4)
                    px, py = int.from_bytes(position[:2], 'little'), int.from_bytes(position[2:], 'little')
                    if (px, py) == (x, y):
                        return
                    emu.request('mouse', dx=max(-255, min(255, x-px)), dy=max(-255, min(255, y-py)))
                    advance(12)
                    ready()
                raise AssertionError('Mouse did not reach its target')

            def click(x, y):
                move(x, y)
                emu.request('mouse', buttons=1)
                advance(12)
                emu.request('mouse', buttons=0)
                advance(180)

            def screenshot(name):
                path = captures / (name + '.png')
                assert emu.screenshot(path)['captured']
                return path

            def quit_puzzle():
                key('Escape', settle=16)
                key('Y')
                ready()

            def status(number):
                offset = int.from_bytes(emu.read_memory(number*2, 2, space='banked_ram', bank=2), 'little')
                shape = emu.read_memory(offset, space='banked_ram', bank=2)[0]
                pointer = offset + 1 + (((shape >> 4) * (shape & 15) + 1)//2)
                known = emu.read_memory(pointer, space='banked_ram', bank=2)[0]
                return emu.read_memory(pointer+1+known, space='banked_ram', bank=2)[0]

            advance(300)
            key('Return')
            ready()
            assert variable('_menu_page') == 0
            # Hovering must preserve the patterned background and shadows.
            move(320,400)
            ready()
            background = (ROOT / 'src/JOURNAL0.DAT').read_bytes() + (ROOT / 'src/JOURNAL1.DAT').read_bytes()
            def footer_pixels():
                return emu.read_memory(416*160,36*160,space='vram')
            normal_footer = footer_pixels()
            for x,width in ((64,22),(160,34),(304,26)):
                for y in range(416,452):
                    offset = y*160+x//4
                    assert emu.read_memory(offset,width,space='vram') == background[offset:offset+width]
            # The final row and shadow padding belong to the background.
            for y in range(416,452):
                for x in (586,587):
                    offset = y*160+x//4
                    shift = 6-(x%4)*2
                    assert (emu.read_memory(offset,space='vram')[0] >> shift) & 3 == (background[offset] >> shift) & 3
            for x in (106,226,354,568):
                move(x,432)
                ready()
                if x == 568:
                    hovered_footer = footer_pixels()
                    for row in range(36):
                        for column in range(424,588):
                            if 4 <= row < 28 and 554 <= column < 582:
                                continue
                            offset = row*160+column//4
                            shift = 6-(column%4)*2
                            assert (hovered_footer[offset] >> shift) & 3 == (normal_footer[offset] >> shift) & 3
                    screenshot('next-page-hover')
                move(320,400)
                ready()
                assert footer_pixels() == normal_footer
            screenshot('footer-restored')
            initial = screenshot('journal')
            with Image.open(initial) as image:
                assert image.size == (640, 480)
                assert len(image.getcolors(640*480)) == 4
            for page in range(1, 4):
                click(568, 432)
                ready()
                assert variable('_menu_page') == page
            screenshot('page-four')
            click(568, 432)  # Disabled on the last page.
            ready()
            assert variable('_menu_page') == 3
            click(512, 304)  # Last slot on the last page is puzzle 96.
            assert variable('_current_puzzle_id') == 95
            assert variable('_gamestate') & 1
            assert emu.read_memory(0x1fa00, 10, space='vram') == (ROOT / 'src/JPALET.DAT').read_bytes()
            screenshot('puzzle-96')
            quit_puzzle()
            assert status(96) & 2
            for page in (2, 1, 0):
                click(440, 432)
                ready()
                assert variable('_menu_page') == page
            click(440, 432)  # Disabled on the first page.
            ready()
            assert variable('_menu_page') == 0

            # Open/leave puzzle 1, then hover puzzle 2 to expose the opened card.
            click(128, 136)
            assert variable('_current_puzzle_id') == 0
            quit_puzzle()
            assert status(1) & 2
            move(200, 136)
            ready()
            screenshot('journal-progress')

            click(226, 432)
            ready()
            screenshot('options')
            # Returning to ON must erase the wider OFF button completely.
            def music_pixels():
                return b''.join(emu.read_memory(y*160+57, 46, space='vram')
                                for y in range(208,244))
            move(308, 224)
            ready()
            music_on = music_pixels()
            click(308, 224)
            ready()
            assert variable('_music') == 0
            click(308, 224)
            ready()
            assert variable('_music') == 1
            assert music_pixels() == music_on
            click(320, 266)
            ready()
            for x, flag in ((106, 16), (354, 128)):
                click(x, 432)
                assert variable('_gamestate') == flag
                assert emu.read_memory(0x1fa00, 10, space='vram') == (ROOT / 'src/JPALET.DAT').read_bytes()
                key('Escape')
                ready()
            move(320, 240)  # Journal gutter does not select a puzzle.
            key('Right', settle=12)
            ready()
            key('Return')
            assert variable('_current_puzzle_id') == 2
            assert emu.read_memory(0x1fa00, 10, space='vram') == (ROOT / 'src/JPALET.DAT').read_bytes()
            quit_puzzle()
            print('PASS: four-color journal, all four pages, boundary navigation, puzzle 96,')
            print('      progress, music options, Help/About, keyboard selection, palette restoration.')
            if args.screenshots:
                print('Screenshots:', captures)


if __name__ == '__main__':
    main()
