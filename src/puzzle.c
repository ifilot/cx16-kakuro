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
uint8_t puzzlecells = 0;
uint8_t offset_x = 0;
uint8_t offset_y = 0;
int8_t ccurx = 0;
int8_t ccury = 0;
int8_t ocurx = 0;
int8_t ocury = 0;

static const uint8_t rambank_puzzle = RAMBANK_PUZZLE;

/**
 * @brief Build puzzle
 * 
 */
void build_puzzle() {
    uint8_t i, j, ctr, c, idx;
    uint8_t *v = (uint8_t*)0xA000;

    // set ram bank to load puzzle data into
    asm("lda %v", rambank_puzzle);
    asm("sta 0");

    // load puzzle into memory
    cbm_k_setnam("puzzle.dat");
    cbm_k_setlfs(0, 8, 2);
    cbm_k_load(0, 0xA000);

    puzzlerows = (*v >> 4) & 0x0F;
    puzzlecols = *v & 0x0F;
    puzzlecells = puzzlerows*puzzlecols;
    puzzledata = (uint8_t*)malloc(puzzlecells);
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

    // swap back to default ram bank
    asm("lda 0");
    asm("sta 0");

    // from here on, all puzzle data is in main memory
    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {
            idx = i * puzzlecols + j;

            // if the tile is a number, ignore it
            if(puzzledata[idx] != 0) {
                continue;
            }

            // only check for right-hand non-zero cells
            if(i >= puzzlerows-2 && j+2 < puzzlecols) {
                if(puzzledata[idx + 1] > 0 && puzzledata[idx + 1] < 0x0A &&
                   puzzledata[idx + 2] > 0 && puzzledata[idx + 2] < 0x0A) {
                    puzzledata[idx] = (1 << 6);
                    continue;
                }
            }

            // only check for bottom cells
            if(j >= puzzlecols-2) {
                if(puzzledata[idx + puzzlecols] > 0 && puzzledata[idx + puzzlecols] < 0x0A &&
                   puzzledata[idx + 2*puzzlecols] > 0 && puzzledata[idx + 2*puzzlecols] < 0x0A) {
                    puzzledata[idx] = (1 << 7);
                    continue;
                }
            }

            if(puzzledata[idx + 1] > 0 && puzzledata[idx + 1] < 0x0A &&
               puzzledata[idx + 2] > 0 && puzzledata[idx + 2] < 0x0A) {
                puzzledata[idx] |= (1 << 6);
            }

            if(puzzledata[idx + puzzlecols] > 0 && puzzledata[idx + puzzlecols] < 0x0A &&
               puzzledata[idx + 2*puzzlecols] > 0 && puzzledata[idx + 2*puzzlecols] < 0x0A) {
                puzzledata[idx] |= (1 << 7);
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
                set_tile(offset_y + i*2+1, offset_x + j*2, TILE_CLUE2, 0x00);
                set_tile(offset_y + i*2, offset_x + j*2+1, TILE_CLUE2, (1 << 2) | (1 << 3));
                set_tile(offset_y + i*2+1, offset_x + j*2+1, TILE_CLUE1, (1 << 2) | (1 << 3));
            }
        }
    }

    ctr = 17;
    // loop over tiles and generate clues
    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {

            // generate right-clue
            idx = i * puzzlecols + j;
            if(puzzledata[idx] & (1 << 6)) {
                c = 0;
                idx++;
                while((puzzledata[idx] & 0x0F) > 0 && idx < puzzlecells) {
                    c += puzzledata[idx];
                    idx++;
                }
                build_clue_tile_right(ctr, c);
                set_tile(offset_y + i*2, offset_x + j*2+1, ctr, 0x00);
                ctr++;
            }

            // generate down clue
            idx = i * puzzlecols + j;
            if(puzzledata[idx] & (1 << 7)) {
                c = 0;
                idx += puzzlecols;
                while((puzzledata[idx] & 0x0F) > 0 && idx < (puzzlecells)) {
                    c += puzzledata[idx];
                    idx += puzzlecols;
                }
                build_clue_tile_down(ctr, c);
                set_tile(offset_y + i*2+1, offset_x + j*2, ctr, 0x00);
                ctr++;
            }
        }
    }
}

void set_puzzle_tile(uint8_t y, uint8_t x, uint8_t tile) {
    set_tile(offset_y + y*2, offset_x + x*2, tile, 0x00);
    set_tile(offset_y + y*2+1, offset_x + x*2, tile, (1 << 3));
    set_tile(offset_y + y*2, offset_x + x*2+1, tile, (1 << 2));
    set_tile(offset_y + y*2+1, offset_x + x*2+1, tile, (1 << 2) | (1 << 3));
}

void puzzle_handle_mouse() {
    static uint8_t mouse_buttons = 0x00;
    uint16_t *mouse_x = (uint16_t *)0x2;
    uint16_t *mouse_y = (uint16_t *)0x4;
    uint8_t d = 0;

    // read mouse
    asm("ldx #2");
    asm("jsr $FF6B");
    asm("sta %v", mouse_buttons);

    // get board position from mouse position
    ccurx = ((*mouse_x >> 4) - offset_x) >> 1;
    ccury = ((*mouse_y >> 4) - offset_y) >> 1;

    if(ccurx != ocurx || ccury != ocury) {
        d = puzzledata[ocury * puzzlecols + ocurx];
        if(d > 0x00 && d < 0x0A) {
            set_puzzle_tile(ocury, ocurx, TILE_EMPTY);
        }
    }

    if(ccurx >= 0 && ccurx < puzzlecols && ccury >=0 && ccury < puzzlerows) {
        d = puzzledata[ccury * puzzlecols + ccurx];
        if(d > 0x00 && d < 0x0A) {
            set_puzzle_tile(ccury, ccurx, TILE_HIGHLIGHT);
            ocurx = ccurx;
            ocury = ccury;
        }
    }
}