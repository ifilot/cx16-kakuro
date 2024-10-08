/**************************************************************************
 *                                                                        *
 *   Author: Ivo Filot <ivo@ivofilot.nl>                                  *
 *                                                                        *
 *   CX16-KAKURO is free software:                                        *
 *   you can redistribute it and/or modify it under the terms of the      *
 *   GNU General Public License as published by the Free Software         *
 *   Foundation, either version 3 of the License, or (at your option)     *
 *   any later version.                                                   *
 *                                                                        *
 *   CX16-KAKURO is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty          *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.              *
 *   See the GNU General Public License for more details.                 *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program.  If not, see http://www.gnu.org/licenses/.  *
 *                                                                        *
 **************************************************************************/

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

// graphics positioning in VRAM
#define TILEBASE_MENU       0x00000      // 6*16 tiles at 8bpp
#define TILEBASE_GAME       0x06000      // 2*16 tiles at 8bpp
#define TILEBASE_CUSTOM     0x0A000      // 4*16 tiles at 8bpp
#define MAPBASE0            0x0D000      // 64 x 64 tiles
#define TILEBASE_FONT16     0x14000      // 10 * 16 tiles at 1bpp
#define TILEBASE_FONT8      0x16000      // 6 * 16 tiles at 1bpp
#define MAPBASE1            0x18000      // 64 x 64 tiles
#define PALETTEBASE         0x1FA20
#define PALETTEBYTE         0x00
#define BANKED_RAM          0xA000

#define MAPHEIGHT           64
#define MAPWIDTH            64

// define tiles
#define TILE_NONE           0x00
#define TILE_BACKGROUND     0x01
#define TILE_BLOCKED        0x02
#define TILE_CLUE1          0x03
#define TILE_CLUE2          0x04
#define TILE_EMPTY          0x05
#define TILE_HIGHLIGHT      0x06
#define TILE_REVEALED       0x07

// mirroring options
#define MIRROR_X            (1 << 2)
#define MIRROR_Y            (1 << 3)
#define MIRROR_XY           (MIRROR_X | MIRROR_Y)

// rambanks
#define RAMBANK_SOUND       0x01
#define RAMBANK_PUZZLE      0x02
#define RAMBANK_TILE        0x03
#define RAMBANK_COLSWAP     0x04
#define RAMBANK_SCREEN      0x05
#define RAMBANK_DOCVIEW     0x06

// puzzledata settings
#define TLDT_REVEALED       (1 << 4)
#define TLDT_LOCKED         (1 << 5)
#define TLDT_HCLUE          (1 << 6)
#define TLDT_VCLUE          (1 << 7)

// layers
#define LAYER0              0
#define LAYER1              1

// userdata settings
#define TLDT_WRITTEN        (1 << 4)

// tile creation
#define SMALL_DIGIT_BUFFER  0xA000
#define TILE_SRC            0xA140
#define TILE_BUFFER         0xA240

// banked memory position
#define BANKED_RAM          0xA000

// game states
#define GAME_PLAY           (1 << 0)
#define GAME_QUIT           (1 << 1)
#define GAME_VERIFY         (1 << 2)
#define GAME_COMPLETE       (1 << 3)
#define GAME_DOCVIEW_EXP    (1 << 4)
#define GAME_OPTIONS        (1 << 5)
#define GAME_DOCVIEW_ABOUT  (1 << 7)

// keycodes
#define KEYCODE_DOWN        0x91
#define KEYCODE_UP          0x11
#define KEYCODE_LEFT        0x9D
#define KEYCODE_RIGHT       0x1D
#define KEYCODE_RETURN      0x0D
#define KEYCODE_SPACE       0x20
#define KEYCODE_ESCAPE      0x1B

// puzzle statuses
#define STATUS_SOLVED       (1 << 0)
#define STATUS_OPENED       (1 << 1)

#define NO                  0x00
#define YES                 0x01

#define MENU_HIGHLIGHT      0x9A
#define MAX_PAGES           2

// menu label positions
#define ML_EXPL             4
#define ML_OPTS             (ML_EXPL + 4 + 2)
#define ML_ABOUT            (ML_OPTS + 7 + 2)
#define ML_PAGE             30

#endif // _CONSTANTS_H
