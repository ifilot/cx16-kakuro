# Changelog

## [0.4.0] — Unreleased

### Changed

- Reduce the distribution to six files: `KAKURO.PRG`, `ASSETS.DAT`,
  `PUZZLE.DAT`, `MENU.ZSM`, `GAME.ZSM`, and `SFX.BIN`.
- Combine graphics, fonts, controls, dialogs, and Help/About content into
  one indexed resource container. Load individual resources into the existing
  RAM and video-memory caches.
- Keep puzzle progress separate and compatible with v0.3.0 saves.
- Check resource-layout compatibility at startup and verify the six-file
  package in the build tests.

## [0.3.0] — 2026-10-09

### Added

- Sakura courtyard start screen.
- Browser version using the Commander X16 WebAssembly emulator, hosted on
  GitHub Pages. Desktop uses a native 640×480 display; fullscreen uses integer
  pixel scaling. Mobile provides panning, tap selection, and an on-screen keypad.
- Root `make run`, `make dist`, `make site`, and `make tilesheets` targets.
- Windows emulator build and launch scripts.
- Technical About information including version, commit, compiler, and settings.

### Changed

- Apply a consistent four-color Japanese journal style to puzzle selection,
  gameplay, Help, About, Options, and exit confirmation.
- Present the existing 96 puzzles across four pages of 24 cards, with blossom
  difficulty indicators and opened/solved markers.
- Make Help and About scrollable and simplify the gameplay sidebar.
- Speed up hover updates with cached graphics and assembly transfers; preload
  menu assets and retain a visible screen during scene transitions.
- Replace the previous audio with **Niwa**, **Quiet Grid**, and 18 sound effects.
- Replace obsolete artwork and editable project files with source PNG tile
  sheets and charmaps connected to the Python asset generators.
- Replace the README animation with screenshots and update build/release Actions.

### Fixed

- Correct the duplicate entry in puzzle 031 in its source data.
- Fix menu hover artifacts, clue-text alignment, blocked-cell diagonals,
  and modal button placement.
- Wait for rendered scenes and pointer calibration in browser tests.
- Fix the release ZIP artifact handoff so tagged releases publish correctly.

## [0.2.0] — 2024-09-13

### Added

- Expand the puzzle collection from 48 to 96, adding 9×9 and 10×10 boards.
- Add menu pagination, puzzle-size labels, and difficulty indicators.
- Add in-game Help and About pages using an 8×8 text font.
- Add confirmation before leaving a puzzle.

### Changed

- Improve the puzzle-selection layout and graphical assets.
- Include Help/About text in the distribution and fix release packaging.

## [0.1.0] — 2024-09-01

### Added

- Initial published Commander X16 Kakuro release with 48 puzzles, from 6×6
  to 8×8.
- Mouse-based cell selection and keyboard digit entry, with generated sum clues
  and highlighted cells.
- Save opened/completed puzzle status, track elapsed time, and notify the player
  when a puzzle is solved.
- Optional verification showing correct and incorrect entries in different colors.
- Background music and sound effects through ZSMKit.
- Build and release automation through GitHub Actions.

[0.4.0]: https://github.com/ifilot/cx16-kakuro/compare/v0.3.0...develop
[0.3.0]: https://github.com/ifilot/cx16-kakuro/releases/tag/v0.3.0
[0.2.0]: https://github.com/ifilot/cx16-kakuro/releases/tag/v0.2.0
[0.1.0]: https://github.com/ifilot/cx16-kakuro/releases/tag/v0.1.0
