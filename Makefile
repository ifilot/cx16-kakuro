EMU ?= $(abspath ../x16-emulator/build/x16emu)
ROM_PATHS = ../emulator/rom.bin $(dir $(EMU))rom.bin ../emulator-win/rom.bin
ROM ?= $(abspath $(firstword $(wildcard $(ROM_PATHS))))
EMU_FLAGS ?= -scale 2
V ?= 0
DEMO_GIF ?= img/cx16-kakuro-gameplay.gif

ifeq ($(V),1)
Q =
else
Q = @
endif

.PHONY: all run demo dist site tilesheets clean

all:
	@printf '[build] Building Kakuro...\n'
ifeq ($(V),1)
	+$(MAKE) --no-print-directory -C src
else
	+@log=$$(mktemp) || exit 1; \
	trap 'rm -f "$$log"' EXIT HUP INT TERM; \
	$(MAKE) --no-print-directory -s -C src >"$$log" || { status=$$?; cat "$$log"; exit $$status; }
endif
	@printf '[build] Ready: src/KAKURO.PRG\n'

run: all
	@test -n "$(ROM)" && test -f "$(ROM)" || { echo 'ROM not found. Use make run ROM=/path/to/rom.bin' >&2; exit 1; }
	@printf '[run] Emulator: %s\n[run] ROM:      %s\n[run] Launching Kakuro...\n' "$(EMU)" "$(ROM)"
	$(Q)cd src && "$(EMU)" -rom "$(ROM)" -prg KAKURO.PRG -run $(EMU_FLAGS)

demo: all
	$(Q)python3 tools/record_gameplay.py --emulator "$(EMU)" --rom "$(ROM)" --output "$(DEMO_GIF)"

dist: all
	mkdir -p build
	rm -f build/CX16-KAKURO.ZIP
	zip -q --junk-paths build/CX16-KAKURO.ZIP src/KAKURO.PRG src/*.DAT src/*.ZSM src/SFX.BIN src/*.TXT
	unzip -tq build/CX16-KAKURO.ZIP
	@echo "Distribution: build/CX16-KAKURO.ZIP"

site: dist
	python3 tools/build_site.py

clean:
	$(MAKE) -C src clean

tilesheets: all
	python3 assets/scripts/export_tilesheets.py
