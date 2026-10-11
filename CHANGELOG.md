# Changelog

## 0.4.0 - 2026-10-11

### Added

- A journal-style puzzle completion card using the small UI font, a blossom
  accent, an elapsed-time display, and a button supporting mouse clicks or Enter.
- A persistent "TURNING THE PAGE" indicator while the next scene loads.
- Automatic gameplay GIF recording with `make demo`, covering puzzle selection,
  solving, completion, and returning to the journal without changing saved progress.
- Instructions for native Windows testing, comparing emulator audio buffer sizes,
  and recording audio to WAV.

### Changed

- Reduce the distribution to six files: `KAKURO.PRG`, `ASSETS.DAT`,
  `PUZZLE.DAT`, `MENU.ZSM`, `GAME.ZSM`, and `SFX.BIN`.
- Combine graphics, fonts, controls, dialogs, and Help/About content into an
  indexed resource container, retaining the existing RAM and video-memory caches.
- Keep puzzle progress separate and compatible with v0.3.0 saves.
- Check resource-layout compatibility at startup and verify the six-file package
  in build and emulator tests.
- Scene transitions now fade smoothly through paper over six frames in each
  direction, using the outgoing scene's palette for the initial fade.
- Build output is concise by default; `V=1` shows full commands and asset output.
- Linux and Windows launchers search additional emulator and ROM locations,
  including the neighboring `emulator-win` installation.
- Completion redraws service the music stream between rows.
- Emulator checks cover completion card pixels, mouse activation, fixed finishing
  times, and the page indicator during loading. Audio, gameplay, and transition
  checks can use the neighboring Windows ROM when the original ROM path is absent.

### Fixed

- Saved completion times no longer include time spent reading the completion card.
- The source asset build check requests verbose output so it can verify which
  generators and compiler commands ran.

## 0.3.0 - 2026-10-09

### Added

- A courtyard title screen and a four-color Sakura Journal interface at 640x480.
- Puzzle cards with difficulty blossoms, opened/solved indicators, and details
  for the selected puzzle, arranged across four journal pages.
- Keyboard navigation alongside mouse controls in the puzzle journal.
- Journal-style Help, About, Options, and quit-confirmation screens, with small
  text, document scrolling, and build information in About.
- Dedicated menu music (**Niwa**) and puzzle music (**Quiet Grid**), plus 18 sound
  effects for navigation, number entry, checking, dialogs, and completion.
- A browser-playable website with fullscreen controls, a downloadable game bundle,
  GitHub Pages deployment, and mobile panning, tap selection, and an on-screen keypad.
- Native Windows launcher and emulator build scripts.
- Root-level build, emulator launch, distribution, website, and tilesheet targets,
  plus automated checks for assets, gameplay, documents, audio, and transitions.

### Changed

- Reworked the playfield with journal scenery, new cell artwork, small clue
  digits, and a Check control showing ticks for correct entries and crosses for
  incorrect entries.
- Replaced the previous music and effects with the generated Kakuro audio bundle.
  Effects remain available while music is muted.
- Cached journal controls and puzzle details in banked RAM and used assembly
  transfers for faster rendering. Board edits update the tile overlay.
- Made source artwork and puzzle changes rebuild their generated assets.
- Added paper-colored fades between scenes and updated screenshots and documentation.

### Fixed

- Corrected the duplicate entry in puzzle 031 and excluded unused empty puzzle
  files from generation.
- Fixed menu hover artifacts, clue-text alignment, blocked-cell diagonals, and
  modal button placement.
- Browser checks wait for scene rendering and mouse calibration before clicking.
- Corrected the release ZIP artifact handoff between build and publication jobs.

## 0.2.0 - 2024-09-13

### Added

- Expanded the playable collection from 48 to 96 puzzles, including 9x9 and 10x10
  boards alongside the existing 6x6, 7x7, and 8x8 sizes.
- Multiple puzzle selection pages, puzzle-size labels, difficulty indicators,
  and metadata for board size and difficulty.
- An 8x8 font and an in-game document viewer for Help and About.
- Confirmation before leaving a puzzle.

### Changed

- Improved menu layout and graphical artwork.
- Updated the gameplay recording.

### Fixed

- Included text documents in the distribution ZIP and corrected deployment and
  release filename handling.

## 0.1.0 - 2024-09-01

### Added

- Initial playable Commander X16 release with 48 puzzles spanning 6x6 to 8x8.
- Mouse-based cell selection, keyboard number entry, highlighted cells, given
  digits, and generated sum clues.
- A puzzle menu with opened and solved status indicators.
- A correctness toggle distinguishing correct and incorrect entries.
- An elapsed-time clock and puzzle completion message.
- Persistent puzzle progress and finishing times; unfinished board entries are
  not saved.
- Background music and number-entry sound through the bundled ZSMKit engine.
- Asset generation scripts, a cc65 build, automated build/release packaging, and
  a gameplay recording.

### Fixed

- Clue generation and number placement issues found during initial development.
