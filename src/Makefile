SOUND_ASSETS=MENU.ZSM GAME.ZSM SFX.BIN
DOCUMENT_ASSETS=DHELP0.DAT DHELP1.DAT DABOUT0.DAT DABOUT1.DAT DHELPT.DAT DABOUTT.DAT DCONTROL.DAT
CC=cl65
CFLAGS?=-O
LDFLAGS?=-t cx16 -C kakuro.cfg
EMU=~/emulator-linux/x16emu
PYTHON=python3
PLAYFIELD_ASSETS=GPLAY0.DAT GPLAY1.DAT GTILES.DAT GCONTROLS.DAT GQUIT.DAT tile_layout.h
JOURNAL_ASSETS=JOURNAL0.DAT JOURNAL1.DAT JPALET.DAT JCURSOR.DAT JUI.DAT JRESTORE.DAT JDIALOG.DAT $(foreach page,1 2 3 4,$(foreach part,0 1 2 3 4 5,JDETAIL$(page)$(part).DAT)) $(foreach page,1 2 3 4,JPAGE$(page)0.DAT JPAGE$(page)1.DAT)

RESOURCE_ASSETS=$(filter-out JPALET.DAT,$(JOURNAL_ASSETS)) $(DOCUMENT_ASSETS) $(filter-out tile_layout.h,$(PLAYFIELD_ASSETS)) FONT8.DAT SPLASH0.DAT SPLASH1.DAT SPLASHP.DAT

make: KAKURO.PRG

KAKURO.PRG: BUILDINFO.json *.c *.h *.s *.lib kakuro.cfg PUZZLE.DAT ASSETS.DAT resource_index.h tile_layout.h $(SOUND_ASSETS) sfx.h
	$(CC) $(CFLAGS) -o KAKURO.PRG -m KAKURO.map -Ln KAKURO.sym $(LDFLAGS) *.c *.s *.lib

PUZZLE.DAT: ../assets/scripts/create_puzzle.py $(wildcard ../assets/puzzles/*.puz)
	$(PYTHON) ../assets/scripts/create_puzzle.py PUZZLE.DAT

FONT8.DAT: ../assets/tiles/ui-charmap.png ../assets/scripts/create_fontmap.py
	$(PYTHON) ../assets/scripts/create_fontmap.py .

$(SOUND_ASSETS) sfx.h: %: ../assets/sound/%
	cp -v $< $@

SPLASH0.DAT SPLASH1.DAT SPLASHP.DAT &: ../assets/backgrounds/start.png ../assets/palette/cx16palette.png ../assets/scripts/create_splash.py
	$(PYTHON) ../assets/scripts/create_splash.py .

$(JOURNAL_ASSETS) &: ../assets/tiles/difficulty.png ../assets/scripts/difficulty.py ../assets/backgrounds/journal.png ../assets/tiles/ui-charmap.png ../assets/scripts/create_menu.py ../assets/scripts/create_puzzle.py $(wildcard ../assets/puzzles/*.puz)
	$(PYTHON) ../assets/scripts/create_menu.py .

run: KAKURO.PRG
	$(EMU) -prg KAKURO.PRG -run

clean:
	rm -f *.o *.DAT *.PRG *.ZSM SFX.BIN sfx.h tile_layout.h resource_index.h

$(PLAYFIELD_ASSETS) &: ../assets/tiles/layout.json ../assets/tiles/cells.png ../assets/tiles/dialog-borders.png ../assets/tiles/difficulty.png ../assets/scripts/difficulty.py ../assets/scripts/create_playfield.py ../assets/scripts/create_menu.py ../assets/backgrounds/journal.png ../assets/tiles/ui-charmap.png ../assets/tiles/clue-charmap.png
	$(PYTHON) ../assets/scripts/create_playfield.py .

$(DOCUMENT_ASSETS) &: BUILDINFO.json HELP.TXT ABOUT.TXT ../assets/scripts/create_documents.py ../assets/scripts/create_menu.py ../assets/backgrounds/journal.png ../assets/tiles/ui-charmap.png
	$(PYTHON) ../assets/scripts/create_documents.py .

.PHONY: FORCE
FORCE:
BUILDINFO.json: FORCE ../VERSION ../assets/scripts/build_info.py
	$(PYTHON) ../assets/scripts/build_info.py $@ "$(CC)" "$(CFLAGS) $(LDFLAGS)"

ASSETS.DAT resource_index.h &: $(RESOURCE_ASSETS) ../assets/scripts/resource_bundle.py
	$(PYTHON) ../assets/scripts/resource_bundle.py . $(RESOURCE_ASSETS)
