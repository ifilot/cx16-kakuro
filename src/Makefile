SOUND_ASSETS=MENU.ZSM GAME.ZSM SFX.BIN
DOCUMENT_ASSETS=DHELP0.DAT DHELP1.DAT DABOUT0.DAT DABOUT1.DAT DHELPT.DAT DABOUTT.DAT DCONTROL.DAT
CC=cl65
CFLAGS?=-O
LDFLAGS?=-t cx16 -C kakuro.cfg
EMU=~/emulator-linux/x16emu
PYTHON=python3
PLAYFIELD_ASSETS=GPLAY0.DAT GPLAY1.DAT GTILES.DAT GCONTROLS.DAT GQUIT.DAT
JOURNAL_ASSETS=JOURNAL0.DAT JOURNAL1.DAT JPALET.DAT JCURSOR.DAT JUI.DAT JRESTORE.DAT JDIALOG.DAT $(foreach page,1 2 3 4,$(foreach part,0 1 2 3 4 5,JDETAIL$(page)$(part).DAT)) $(foreach page,1 2 3 4,JPAGE$(page)0.DAT JPAGE$(page)1.DAT)

make: KAKURO.PRG

KAKURO.PRG: BUILDINFO.json *.c *.h *.s *.lib kakuro.cfg TILES.DAT MTILES.DAT PUZZLE.DAT SDIGITS.DAT FONT16.DAT FONT8.DAT $(SOUND_ASSETS) sfx.h SPLASH0.DAT SPLASH1.DAT SPLASHP.DAT $(JOURNAL_ASSETS) $(PLAYFIELD_ASSETS) $(DOCUMENT_ASSETS)
	$(CC) $(CFLAGS) -o KAKURO.PRG -m KAKURO.map -Ln KAKURO.sym $(LDFLAGS) *.c *.s *.lib

TILES.DAT: ../assets/tiles/tiles.png ../assets/scripts/convert_tilemap.py
	$(PYTHON) ../assets/scripts/convert_tilemap.py tiles.png TILES.DAT 5

MTILES.DAT: ../assets/tiles/menu-tiles.png ../assets/scripts/convert_tilemap.py
	$(PYTHON) ../assets/scripts/convert_tilemap.py menu-tiles.png MTILES.DAT 6

PUZZLE.DAT: ../assets/scripts/create_puzzle.py $(wildcard ../assets/puzzles/*.puz)
	$(PYTHON) ../assets/scripts/create_puzzle.py PUZZLE.DAT

SDIGITS.DAT:
	$(PYTHON) ../assets/scripts/convert_small_digits.py SDIGITS.DAT

FONT16.DAT FONT8.DAT: ../assets/tiles/font-tiles-8.png ../assets/tiles/font-tiles-16.png  ../assets/scripts/create_fontmap.py
	$(PYTHON) ../assets/scripts/create_fontmap.py

$(SOUND_ASSETS) sfx.h: %: ../assets/sound/%
	cp -v $< $@

SPLASH0.DAT SPLASH1.DAT SPLASHP.DAT &: ../assets/splash/comparisons/courtyard-640x480-4colors.png ../assets/palette/cx16palette.png ../assets/scripts/create_splash.py
	$(PYTHON) ../assets/scripts/create_splash.py .

$(JOURNAL_ASSETS) &: ../assets/scripts/difficulty.py ../assets/menu/concepts/03-puzzle-journal-source.png ../assets/tiles/font-tiles-8.png ../assets/scripts/create_menu.py ../assets/scripts/create_puzzle.py $(wildcard ../assets/puzzles/*.puz)
	$(PYTHON) ../assets/scripts/create_menu.py .

run: KAKURO.PRG
	$(EMU) -prg KAKURO.PRG -run

clean:
	rm -f *.o *.DAT *.PRG *.ZSM SFX.BIN sfx.h

$(PLAYFIELD_ASSETS) &: ../assets/scripts/difficulty.py ../assets/scripts/create_playfield.py ../assets/scripts/create_menu.py ../assets/menu/concepts/03-puzzle-journal-source.png ../assets/tiles/font-tiles-8.png ../assets/tiles/small-digits.png
	$(PYTHON) ../assets/scripts/create_playfield.py .

$(DOCUMENT_ASSETS) &: BUILDINFO.json HELP.TXT ABOUT.TXT ../assets/scripts/create_documents.py ../assets/scripts/create_menu.py ../assets/menu/concepts/03-puzzle-journal-source.png ../assets/tiles/font-tiles-8.png
	$(PYTHON) ../assets/scripts/create_documents.py .

.PHONY: FORCE
FORCE:
BUILDINFO.json: FORCE ../VERSION ../assets/scripts/build_info.py
	$(PYTHON) ../assets/scripts/build_info.py $@ "$(CC)" "$(CFLAGS) $(LDFLAGS)"
