"""Measure menu loop CPU cycles, independent of emulator host speed."""
import argparse
from pathlib import Path
import re
import shutil
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--max-cycles', type=int)
    parser.add_argument('--progress', action='store_true', help='Benchmark opened and solved cards too')
    parser.add_argument('--emulator', type=Path, default=ROOT.parent / 'x16-emulator/build/x16emu')
    parser.add_argument('--rom', type=Path, default=ROOT.parent / 'emulator/rom.bin')
    args = parser.parse_args()
    sys.path.insert(0, str(ROOT.parent / 'x16-emulator/api/python'))
    from x16agent import X16Agent
    symbols = dict((name, int(address, 16)) for address, name in re.findall(
        r'al ([0-9A-Fa-f]+) \.(\S+)', (ROOT / 'src/KAKURO.sym').read_text()))
    with tempfile.TemporaryDirectory() as directory:
        runtime = Path(directory)
        for pattern in ('*.DAT', '*.ZSM', '*.BIN', '*.TXT', 'KAKURO.PRG'):
            for source in (ROOT / 'src').glob(pattern):
                shutil.copy2(source, runtime / source.name)
        if args.progress:
            path = runtime / 'PUZZLE.DAT'
            data = bytearray(path.read_bytes())
            for slot in range(24):
                start = 2 + int.from_bytes(data[4+slot*2:6+slot*2], 'little')
                shape = data[start]
                known = start + 1 + (((shape >> 4) * (shape & 15) + 1)//2)
                status = known + 1 + data[known]
                data[status] |= 2 if slot % 2 == 0 else 1
            path.write_bytes(data)
        with (runtime / 'log').open('w') as log, X16Agent(
                args.emulator.resolve(), args.rom.resolve(),
                fsroot=runtime, options=['-prg', str(runtime / 'KAKURO.PRG'), '-run'], stderr=log) as emu:
            entry = symbols['_menu_handle_mouse']
            def ready():
                emu.request('breakpoint', address=entry)
                for _ in range(10):
                    result = emu.advance(100000000, unit='cycles')
                    if result['reason'] == 'breakpoint': return
                raise AssertionError(result)
            def loop():
                emu.request('breakpoint', action='clear')
                first = emu.advance(1, unit='instructions')['cycles']
                emu.request('breakpoint', address=entry)
                result = emu.advance(10000000, unit='cycles')
                assert result['reason'] == 'breakpoint', result
                return first + result['cycles']
            def move(x,y):
                # Deliver PS/2 input, then sample complete menu iterations for
                # three frames. Include IRQ/music cost in the measured budget.
                position=emu.read_memory(2,4)
                px=int.from_bytes(position[:2],'little');py=int.from_bytes(position[2:],'little')
                while (px,py)!=(x,y):
                    dx=max(-255,min(255,x-px));dy=max(-255,min(255,y-py))
                    emu.request('mouse',dx=dx,dy=dy)
                    px+=dx;py+=dy
                total=0;peak=0
                while total<400000:
                    cost=loop();total+=cost;peak=max(peak,cost)
                print(f'{x},{y}: {peak:,} cycles ({peak/8000:.2f} ms at 8 MHz)',flush=True)
                if args.max_cycles: assert peak <= args.max_cycles, (x,y,peak)
            emu.advance(300)
            emu.request('key', key='Return', down=True);emu.advance(8)
            emu.request('key', key='Return', down=False)
            ready()
            for point in [(128,136),(200,136),(272,136),(106,432),(226,432),(354,432),(568,432)]:
                move(*point)
            emu.request('mouse',dx=-255,dy=-208)
            emu.request('breakpoint',action='clear');emu.advance(20);ready()
            # Open options, then measure both option hover paths.
            move(226,432)
            emu.request('mouse',buttons=1)
            emu.request('breakpoint',action='clear');emu.advance(8)
            emu.request('mouse',buttons=0);emu.advance(20);ready()
            for point in [(308,224),(320,266),(480,300)]:move(*point)

if __name__ == '__main__':main()
