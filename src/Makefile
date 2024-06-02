CC=cl65
EMU=~/emulator-linux/x16emu
PYTHON=python3

make: KAKURO.PRG

KAKURO.PRG: *.c *.h TILES.DAT PUZZLE.DAT SDIGITS.DAT
	$(CC) -O -o KAKURO.PRG -t cx16 -C kakuro.cfg *.c

TILES.DAT:
	$(PYTHON) ../assets/scripts/convert_tilemap.py TILES.DAT

PUZZLE.DAT:
	$(PYTHON) ../assets/scripts/create_puzzle.py PUZZLE.DAT

SDIGITS.DAT:
	$(PYTHON) ../assets/scripts/convert_small_digits.py SDIGITS.DAT

run: KAKURO.PRG
	$(EMU) -prg KAKURO.PRG -run

clean:
	rm -v *.o *.DAT *.PRG