EMU ?= $(abspath ../x16-emulator/build/x16emu)
ROM ?= $(abspath ../emulator/rom.bin)
EMU_FLAGS ?= -scale 2

.PHONY: all run tilesheets clean

all:
	$(MAKE) -C src

run: all
	cd src && "$(EMU)" -rom "$(ROM)" -prg KAKURO.PRG -run $(EMU_FLAGS)

clean:
	$(MAKE) -C src clean

tilesheets: all
	python3 assets/scripts/export_tilesheets.py
