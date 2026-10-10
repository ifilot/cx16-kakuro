"""Record real emulator input and video as a 640x480, 10 fps gameplay GIF."""
import argparse
from itertools import combinations, permutations
import math
from pathlib import Path
import random
import re
import shutil
import sys
import tempfile

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]


def deduction_order(data, cols, rows):
    """Use clue sums and given digits to find forced entries at intersections."""
    runs = []
    lines = [range(y * cols, (y + 1) * cols) for y in range(rows)]
    lines += [range(x, rows * cols, cols) for x in range(cols)]
    for line in lines:
        run = []
        for index in [*line, None]:
            if index is not None and data[index] & 15:
                run.append(index)
            elif run:
                runs.append(run)
                run = []
    # The solution bytes give the displayed clue totals. Individual hidden
    # answers are never used to choose an entry: only sums and revealed givens.
    options = []
    for run in runs:
        total = sum(data[index] & 15 for index in run)
        options.append([order for digits in combinations(range(1, 10), len(run))
                        if sum(digits) == total for order in permutations(digits)])
    known = {index: value & 15 for index, value in enumerate(data) if value & 0x10}
    remaining = {index for index, value in enumerate(data)
                 if not value & 0x30 and value & 15}
    result = []
    previous = None
    while remaining:
        domains = {index: set(range(1, 10)) for index in remaining}
        for run, candidates in zip(runs, options):
            compatible = [order for order in candidates
                          if all(order[pos] == known[index] for pos, index in enumerate(run)
                                 if index in known)]
            for pos, index in enumerate(run):
                if index in remaining:
                    domains[index] &= {order[pos] for order in compatible}
        forced = [index for index, digits in domains.items() if len(digits) == 1]
        if not forced:
            raise RuntimeError('Demo puzzle requires guessing; choose a puzzle with forced deductions')

        def priority(index):
            related = [run for run in runs if index in run]
            # Follow the current clue group, then prefer almost finished,
            # short groups. This naturally alternates across and down.
            return (previous is not None and not any(previous in run for run in related),
                    min((sum(cell in remaining for cell in run), len(run)) for run in related),
                    index)

        index = min(forced, key=priority)
        digit = next(iter(domains[index]))
        known[index] = digit
        remaining.remove(index)
        result.append((index, digit))
        previous = index
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--emulator', type=Path, required=True)
    parser.add_argument('--rom', type=Path, required=True)
    parser.add_argument('--api-dir', type=Path,
                        help='Directory containing the AgentBridge x16agent.py client')
    parser.add_argument('--output', type=Path, default=ROOT / 'img/cx16-kakuro-gameplay.gif')
    args = parser.parse_args()
    api = args.api_dir or args.emulator.resolve().parent.parent / 'api/python'
    sys.path.insert(0, str(api))
    try:
        from x16agent import X16Agent
    except ImportError:
        parser.error('AgentBridge Python client not found; supply --api-dir')
    symbols = {name: int(address, 16) for address, name in re.findall(
        r'al ([0-9A-Fa-f]+) \.(\S+)', (ROOT / 'src/KAKURO.sym').read_text())}
    frames = []
    motion = random.Random(16)
    with tempfile.TemporaryDirectory(prefix='kakuro-demo-') as directory:
        runtime = Path(directory)
        # The game saves puzzle progress; only let it write to disposable copies.
        for pattern in ('*.DAT', '*.ZSM', '*.BIN', '*.TXT', 'KAKURO.PRG'):
            for source in (ROOT / 'src').glob(pattern):
                shutil.copy2(source, runtime / source.name)
        with X16Agent(args.emulator.resolve(), args.rom.resolve(), fsroot=runtime,
                      options=['-prg', str(runtime / 'KAKURO.PRG'), '-run']) as emu:
            def read(name, size=1):
                return int.from_bytes(emu.read_memory(symbols['_' + name], size), 'little')

            def capture():
                emu.screenshot(runtime / 'frame.png')
                with Image.open(runtime / 'frame.png') as frame:
                    assert frame.size == (640, 480), frame.size
                    frames.append(frame.convert('RGB'))

            def record(count):
                for _ in range(count):
                    # Capture itself advances two frames; six emulated frames
                    # per sample give approximately 10 fps at normal game speed.
                    emu.advance(4)
                    capture()

            def key(name):
                emu.request('key', key=name, down=True)
                record(1)
                emu.request('key', key=name, down=False)
                record(1)

            def move(x, y):
                position = emu.read_memory(2, 4)
                start_x = int.from_bytes(position[:2], 'little')
                start_y = int.from_bytes(position[2:], 'little')
                dx, dy = x - start_x, y - start_y
                distance = math.hypot(dx, dy)
                ticks = 3 if distance > 150 else 2
                bend = motion.choice((-1, 1)) * min(24, max(4, distance * motion.uniform(.10, .20)))
                skew = motion.uniform(.85, 1.15)
                px, py = start_x, start_y
                for tick in range(1, ticks + 1):
                    # Feed a curved, eased path at emulator-frame intervals;
                    # the GIF still samples every six frames at 10 fps.
                    for step in range(1, 5):
                        t = ((tick - 1) + step / 4) / ticks
                        eased = t ** skew
                        eased = eased * eased * (3 - 2 * eased)
                        arc = bend * math.sin(math.pi * t)
                        target_x = round(start_x + dx * eased - dy / max(1, distance) * arc)
                        target_y = round(start_y + dy * eased + dx / max(1, distance) * arc)
                        emu.request('mouse', dx=max(-255, min(255, target_x - px)),
                                    dy=max(-255, min(255, target_y - py)))
                        # Relative packets may still be queued in the PS/2
                        # driver. Track sent coordinates to avoid overshooting.
                        px, py = target_x, target_y
                        emu.advance(1)
                    capture()

            def click():
                emu.request('mouse', buttons=1)
                record(1)
                emu.request('mouse', buttons=0)
                record(1)

            print('[demo] Recording title and puzzle selection...', flush=True)
            emu.advance(300)
            record(20)
            key('Return')
            record(15)
            move(128, 136)
            record(6)
            move(200, 136)
            record(6)
            click()
            record(15)
            assert read('current_puzzle_id') == 1, 'Second puzzle was not selected'
            assert read('gamestate') & 1, 'Game did not start'

            print('[demo] Solving through clue groups and intersections...', flush=True)
            cols, rows = read('puzzlecols'), read('puzzlerows')
            data = emu.read_memory(read('puzzledata', 2), cols * rows)
            cells = deduction_order(data, cols, rows)
            assert cells, 'Expected writable cells'
            for index, digit in cells:
                y, x = divmod(index, cols)
                move(read('offset_x') * 16 + x * 32 + 16,
                     read('offset_y') * 16 + y * 32 + 16)
                record(2)
                assert not read('gamestate') & 4, 'Check must stay off while solving'
                key(str(digit))
                record(2)
                actual = emu.read_memory(read('userdata', 2) + index)[0] & 15
                assert actual == digit, f'Digit input failed at cell {index}'
            assert read('tiles_incorrect', 2) == 0, 'Puzzle was not completed'
            print('[demo] Recording congratulations and return to menu...', flush=True)
            record(25)
            key('Return')
            record(8)
            move(200, 136)
            record(15)
            assert read('gamestate') == 0, 'Did not return to puzzle menu'
            saved = (runtime / 'PUZZLE.DAT').read_bytes()
            start = 2 + int.from_bytes(saved[6:8], 'little')
            shape = saved[start]
            givens = start + 1 + (((shape >> 4) * (shape & 15) + 1) // 2)
            status = givens + 1 + saved[givens]
            assert saved[status] & 2, 'Completed puzzle was not marked solved'

    print('[demo] Encoding GIF...', flush=True)
    # A shared palette prevents color flicker; dithering is unnecessary for
    # the original pixel graphics. Sampling is only for palette selection.
    samples = Image.new('RGB', (160 * 8, 120 * ((len(frames) + 7) // 8)))
    for index, frame in enumerate(frames):
        samples.paste(frame.resize((160, 120), Image.Resampling.NEAREST),
                      ((index % 8) * 160, (index // 8) * 120))
    palette = samples.quantize(colors=256, dither=Image.Dither.NONE)
    indexed = [frame.quantize(palette=palette, dither=Image.Dither.NONE) for frame in frames]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    indexed[0].save(args.output, save_all=True, append_images=indexed[1:],
                    duration=100, loop=0, optimize=True, disposal=1)
    with Image.open(args.output) as gif:
        assert gif.size == (640, 480)
        duration = 0
        for index in range(gif.n_frames):
            gif.seek(index)
            delay = gif.info['duration']
            assert delay >= 100 and delay % 100 == 0
            duration += delay
        assert duration == len(frames) * 100
    print(f'[demo] Saved {args.output} ({duration / 1000:.1f}s, 640x480, 10 fps)')


if __name__ == '__main__':
    main()
