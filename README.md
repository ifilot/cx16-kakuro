# CX16-KAKURO

![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/ifilot/cx16-kakuro?label=version)
[![build](https://github.com/ifilot/cx16-kakuro/actions/workflows/build.yml/badge.svg)](https://github.com/ifilot/cx16-kakuro/actions/workflows/build.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

[Download latest version](https://github.com/ifilot/cx16-KAKURO/releases/latest/download/CX16-KAKURO.ZIP)

**Puzzle selection**

![Puzzle journal with puzzle cards and difficulty indicators](img/cx16-kakuro-journal-menu.png)

**Gameplay**

![Kakuro puzzle with sum clues, entered digits and game controls](img/cx16-kakuro-journal-game.png)

## Description

Kakuro is a logic-based number puzzle game, often described as a cross between a
crossword and Sudoku. In this game, players fill a grid with digits from 1 to 9,
with the objective of matching the sum of numbers in each row or column to a
given target. However, no number can be repeated within a single sum. Each clue
is represented as a small number in a dark cell, dictating the sum of the
digits to be placed in the adjacent light cells.

For the Commander X16, Kakuro offers a nostalgic experience, blending the
puzzle's classic challenge with the retro charm of 8-bit computing. Players
navigate the grid using the mouse and inputting numbers via keyboard. The game
tests mathematical reasoning and strategic thinking, making it an engaging
pastime for puzzle enthusiasts and retro gaming fans alike. The simplistic yet
captivating design ensures that Kakuro on the Commander X16 is both a mental
workout and a tribute to vintage gaming.

## Features

* In total, **96** puzzles are implemented in the game varying between 6x6 to
  10x10 board sizes.
* The program keeps track of the user progression. Different colors are used to
  indicate opened and completed puzzles. The game automatically saves the result
  to `PUZZLE.DAT` ensuring 'continuous play'. Note that the game state itself
  is not saved.
* If the user is stuck, they can use the **Check** toggle. Correct entries
  receive a small check; incorrect entries are shown in rose with a cross.

## Compilation

First, install the required dependencies

```bash
sudo apt-get install -y build-essential cc65 python3 python3-numpy python3-pil zip unzip
```
Compilation is fairly straightforward. Run `make` from the repository root.

```bash
make
```

Use `make run` to build and launch the emulator. The default emulator is
`../x16-emulator/build/x16emu`. The ROM is selected from `../emulator/rom.bin`,
`rom.bin` beside the emulator, or `../emulator-win/rom.bin`, in that order.
Override these with `make run EMU=/path/to/x16emu ROM=/path/to/rom.bin` if needed.
The build prints concise status messages and keeps warnings and errors visible.
Use `make run V=1` to see the full build commands and asset generation output.

Use `make dist` to bundle the program and its assets in `build/CX16-KAKURO.ZIP`.

Use `make demo` to automatically record `img/cx16-kakuro-gameplay.gif` at
640×480 and 10 frames per second. It shows the title, puzzle selection,
solving through clue groups and intersections, completion,
and returning to the menu. The approximately 25-second sequence uses curved,
eased mouse movements. Recording uses the neighboring AgentBridge emulator
and its Python client, with Pillow for GIF encoding.
It runs without a window and uses temporary game files to preserve your saves.
The `EMU` and `ROM` overrides also apply; set `DEMO_GIF=/path/to/movie.gif`
to choose another output location. A relocated Python client can be supplied
with `--api-dir` when running the script directly:

```bash
python3 tools/record_gameplay.py --emulator /path/to/x16emu \
  --rom /path/to/rom.bin --api-dir /path/to/api/python
```

## Dependencies

CX16-KAKURO makes use of [zsmkit](https://github.com/mooinglemur/zsmkit) which
is an advanced music and sound effects engine for the Commander X16 and
available under a MIT License. A static copy of this library is bundled in
this repository and automatically embedded in the `.PRG` file.

## Assets

* The original bitmap glyphs and cell artwork were created by me. No
  permission is required to use this artwork in your work, although attribution
  is always greatly appreciated.
* The courtyard and journal backgrounds were generated with OpenAI's image
  generator.
* Music and sound effects were created with `cx16-sound-generator` for Kakuro.
  **Niwa** plays in the menus and **Quiet Grid** during puzzles.

## Community guidelines

* Contributions to CX16-Kakuro are always welcome and appreciated. Before doing so,
  please first read the [CONTRIBUTING](CONTRIBUTING.md) guide.
* For reporting issues or problems with the software, you are kindly invited to
  to open a [new issue with the bug label](https://github.com/ifilot/cx16-kakuro/issues/new?labels=bug).
* If you seek support in using CX16-Kakuro, please 
  [open an issue with the question](https://github.com/ifilot/cx16-kakuro/issues/new?labels=question)
  label.
* If you wish to contact the developers, please send an e-mail to ivo@ivofilot.nl.

## License

Unless otherwise stated, all code in this repository is provided under the GNU
General Public License version 3.
