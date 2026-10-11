"""Copy only files that are shipped, so loose build assets cannot hide loader bugs."""
import shutil

RUNTIME_FILES = ('KAKURO.PRG', 'ASSETS.DAT', 'PUZZLE.DAT', 'MENU.ZSM', 'GAME.ZSM', 'SFX.BIN')


def copy_runtime(source, destination):
    for name in RUNTIME_FILES:
        shutil.copy2(source / name, destination / name)
