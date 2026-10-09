EMU ?= $(abspath ../x16-emulator/build/x16emu)
ROM ?= $(abspath ../emulator/rom.bin)
EMU_FLAGS ?= -scale 2

.PHONY: all run dist site tilesheets clean

all:
	$(MAKE) -C src

run: all
	cd src && "$(EMU)" -rom "$(ROM)" -prg KAKURO.PRG -run $(EMU_FLAGS)

dist: all
	mkdir -p build
	rm -f build/CX16-KAKURO.ZIP
	zip -q --junk-paths build/CX16-KAKURO.ZIP src/KAKURO.PRG src/ASSETS.DAT src/PUZZLE.DAT src/MENU.ZSM src/GAME.ZSM src/SFX.BIN
	unzip -tq build/CX16-KAKURO.ZIP
	@echo "Distribution: build/CX16-KAKURO.ZIP"

site: dist
	python3 tools/build_site.py

clean:
	$(MAKE) -C src clean

tilesheets: all
	python3 assets/scripts/export_tilesheets.py
