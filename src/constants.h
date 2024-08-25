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
#define TILEBASE            0x0000      // 6*16 tiles at 8bpp
#define FONTBASE            0x6000      // 4*16 tiles at 8bpp
#define MAPBASE0            0xA000      // 64 x 64 tiles
#define MAPBASE1            0xB000      // 64 x 64 tiles
#define PALETTEBASE         0x1FA20
#define PALETTEBYTE         0x00

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
#define TILE_BTN_EDGE_UD    0x07
#define TILE_BTN_CORNER     0x08
#define TILE_BTN_CTR        0x09
#define TILE_BTN_EDGE_LR    0x0A

// mirroring options
#define MIRROR_X            (1 << 2)
#define MIRROR_Y            (1 << 3)
#define MIRROR_XY           (MIRROR_X | MIRROR_Y)

// rambanks
#define RAMBANK_PUZZLE      0x01
#define RAMBANK_TILE        0x02
#define RAMBANK_COLSWAP     0x03

// puzzledata settings
#define TLDT_REVEALED       (1 << 4)
#define TLDT_LOCKED         (1 << 5)
#define TLDT_HCLUE          (1 << 6)
#define TLDT_VCLUE          (1 << 7)

// userdata settings
#define TLDT_WRITTEN        (1 << 4)

// tile creation
#define SMALL_DIGIT_BUFFER  0xA000
#define TILE_SRC            0xA140
#define TILE_BUFFER         0xA240

// banked memory position
#define BANKED_RAM          0xA000

#endif // _CONSTANTS_H
