CC=cl65
EMU=~/emulator-linux/x16emu
PYTHON=python3

make: KAKURO.PRG

KAKURO.PRG: *.c *.h *.s TILES.DAT MTILES.DAT PUZZLE.DAT SDIGITS.DAT FONT16.DAT FONT8.DAT KAKURO.ZSM TILE.ZSM
	$(CC) -O -o KAKURO.PRG -t cx16 -C kakuro.cfg *.c *.s *.lib

TILES.DAT: ../assets/tiles/tiles.png ../assets/scripts/convert_tilemap.py
	$(PYTHON) ../assets/scripts/convert_tilemap.py tiles.png TILES.DAT 5

MTILES.DAT: ../assets/tiles/menu-tiles.png ../assets/scripts/convert_tilemap.py
	$(PYTHON) ../assets/scripts/convert_tilemap.py menu-tiles.png MTILES.DAT 6

PUZZLE.DAT: ../assets/scripts/create_puzzle.py ../assets/puzzles/001.puz
	$(PYTHON) ../assets/scripts/create_puzzle.py PUZZLE.DAT

SDIGITS.DAT:
	$(PYTHON) ../assets/scripts/convert_small_digits.py SDIGITS.DAT

FONT16.DAT FONT8.DAT: ../assets/tiles/font-tiles-8.png ../assets/tiles/font-tiles-16.png  ../assets/scripts/create_fontmap.py
	$(PYTHON) ../assets/scripts/create_fontmap.py

KAKURO.ZSM: ../assets/sound/cor.zsm
	cp -v ../assets/sound/cor.zsm KAKURO.ZSM

TILE.ZSM: ../assets/sound/tile.zsm
	cp -v ../assets/sound/tile.zsm TILE.ZSM

run: KAKURO.PRG
	$(EMU) -prg KAKURO.PRG -run

clean:
	rm -v *.o *.DAT *.PRG