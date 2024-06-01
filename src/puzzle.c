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

#include "puzzle.h"

uint8_t* puzzledata = NULL;
uint8_t puzzlerows = 0;
uint8_t puzzlecols = 0;

/**
 * @brief Build puzzle
 * 
 */
void build_puzzle() {
    uint8_t i, j, ctr, c, idx;
    uint8_t offset_x, offset_y;
    uint8_t *v = (uint8_t*)0xA000;

    // load puzzle into memory
    cbm_k_setnam("puzzle.dat");
    cbm_k_setlfs(0, 8, 2);
    cbm_k_load(0, 0xA000);

    puzzlerows = (*v >> 4) & 0x0F;
    puzzlecols = *v & 0x0F;
    puzzledata = (uint8_t*)malloc(puzzlerows*puzzlecols);
    v++;

    offset_x = 20 - puzzlecols;
    offset_y = 15 - puzzlerows;

    if((puzzlerows * puzzlecols) % 2 == 1) {
        offset_x++;
        offset_y++;
    }

    ctr = 0;
    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {
            switch(ctr) {
                case 0:
                    c = (*v >> 4) & 0x0F;
                break;
                case 1:
                    c = *v & 0x0F;
                break;
            }

            puzzledata[i * puzzlecols + j] = c;

            ctr++;
            if(ctr == 2) {
                ctr = 0;
                v++;
            }
        }
    }

    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {
            idx = i * puzzlecols + j;

            // if the tile is a number, ignore it
            if(puzzledata[idx] != 0) {
                continue;
            }

            // only check for right-hand non-zero cells
            if(i >= puzzlerows-3 && j+2 < puzzlecols) {
                if(puzzledata[idx + 1] > 0 && puzzledata[idx + 1] < 0x0A &&
                   puzzledata[idx + 2] > 0 && puzzledata[idx + 2] < 0x0A) {
                    puzzledata[idx] = (1 << 6);
                    continue;
                }
            }

            // only check for bottom cells
            if(j >= puzzlecols-3) {
                if(puzzledata[idx + puzzlecols] > 0 && puzzledata[idx + puzzlecols] < 0x0A &&
                   puzzledata[idx + 2*puzzlecols] > 0 && puzzledata[idx + 2*puzzlecols] < 0x0A) {
                    puzzledata[idx] = (1 << 7);
                    continue;
                }
            }

            if(puzzledata[idx + 1] > 0 && puzzledata[idx + 1] < 0x0A &&
               puzzledata[idx + 2] > 0 && puzzledata[idx + 2] < 0x0A) {
                puzzledata[idx] = (1 << 6);
            }

            if(puzzledata[idx + puzzlecols] > 0 && puzzledata[idx + puzzlecols] < 0x0A &&
               puzzledata[idx + 2*puzzlecols] > 0 && puzzledata[idx + 2*puzzlecols] < 0x0A) {
                puzzledata[idx] = (1 << 7);
            }
        }
    }

    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {
            c = puzzledata[i * puzzlecols + j];

            if(c == 0) {
                set_tile(offset_y + i*2, offset_x + j*2, TILE_BLOCKED, 0x00);
                set_tile(offset_y + i*2+1, offset_x + j*2, TILE_BLOCKED, (1 << 3));
                set_tile(offset_y + i*2, offset_x + j*2+1, TILE_BLOCKED, (1 << 2));
                set_tile(offset_y + i*2+1, offset_x + j*2+1, TILE_BLOCKED, (1 << 2) | (1 << 3));
            } else if(c > 0 && c < 0x0A) {
                set_tile(offset_y + i*2, offset_x + j*2, TILE_EMPTY, 0x00);
                set_tile(offset_y + i*2+1, offset_x + j*2, TILE_EMPTY, (1 << 3));
                set_tile(offset_y + i*2, offset_x + j*2+1, TILE_EMPTY, (1 << 2));
                set_tile(offset_y + i*2+1, offset_x + j*2+1, TILE_EMPTY, (1 << 2) | (1 << 3));
            } else {
                set_tile(offset_y + i*2, offset_x + j*2, TILE_CLUE1, 0x00);
                set_tile(offset_y + i*2+1, offset_x + j*2, TILE_CLUE2, (1 << 2) | (1 << 3));
                set_tile(offset_y + i*2, offset_x + j*2+1, TILE_CLUE2, 0x00);
                set_tile(offset_y + i*2+1, offset_x + j*2+1, TILE_CLUE1, (1 << 2) | (1 << 3));
            }
        }
    }
}