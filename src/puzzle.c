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
uint8_t* userdata = NULL;
uint8_t puzzlerows = 0;
uint8_t puzzlecols = 0;
uint8_t puzzlecells = 0;
uint8_t offset_x = 0;
uint8_t offset_y = 0;
int8_t ccurx = 0;
int8_t ccury = 0;
int8_t ocurx = 0;
int8_t ocury = 0;
uint16_t tiles_incorrect = 0;
uint16_t data_offset = 0;
uint8_t current_puzzle_id = 0;

static const uint8_t rambank_puzzle = RAMBANK_PUZZLE;

/**
 * @brief Build puzzle
 * 
 * PUZZLEDATA FORMATTING
 * =====================
 * 
 *  76543210
 *  ||||\\\\__ value of tile
 *  |||\______ whether cell is revealed
 *  ||\_______ whether cell is locked
 *  |\________ whether cell contains horizontal clue
 *  \_________ whether cell contains vertical clue
 */
void build_puzzle(uint8_t puzzle_id) {
    uint8_t i, j, ctr, c, idx;
    uint8_t nrknowns = 0;
    uint8_t nrcells = 0;
    uint8_t *v;

    // reset incorrect tile counter
    tiles_incorrect = 0;

    // set ram bank to load puzzle data into
    asm("lda %v", rambank_puzzle);
    asm("sta 0");

    // load puzzle into memory
    cbm_k_setnam("puzzle.dat");
    cbm_k_setlfs(0, 8, 2);
    cbm_k_load(0, 0xA000);

    data_offset = *(uint16_t*)(0xA000 + (puzzle_id+1) * 2);
    v = (uint8_t*)(0xA000 + data_offset);

    // sprintf(buf, "%04X", data_offset);
    // write_debug(buf);
    // return;

    puzzlerows = (*v >> 4) & 0x0F;
    puzzlecols = *v & 0x0F;
    puzzlecells = puzzlerows * puzzlecols;
    puzzledata = (uint8_t*)malloc(puzzlecells);
    userdata = (uint8_t*)calloc(puzzlecells, 1);
    v++;

    offset_x = 20 - puzzlecols;
    offset_y = 15 - puzzlerows;

    if((puzzlerows * puzzlecols) % 2 == 1) {
        offset_x++;
        offset_y++;
    }

    // print puzzle id
    set_tile(offset_y + puzzlerows * 2, offset_x + puzzlecols * 2 - 4, 
             0x4E, MIRROR_X, 0x00);
    set_tile(offset_y + puzzlerows * 2, offset_x + puzzlecols * 2 - 3, 
             ((puzzle_id+1) / 10) + 0x44, 0x00, 0x00);
    set_tile(offset_y + puzzlerows * 2, offset_x + puzzlecols * 2 - 2, 
             ((puzzle_id+1) % 10) + 0x44, 0x00, 0x00);
    set_tile(offset_y + puzzlerows * 2, offset_x + puzzlecols * 2 - 1, 
             0x4E, 0x00, 0x00);
    set_tile(offset_y + puzzlerows * 2 + 1, offset_x + puzzlecols * 2 - 4, 
             0x4F, MIRROR_X, 0x00);
    set_tile(offset_y + puzzlerows * 2 + 1, offset_x + puzzlecols * 2 - 3, 
             0x54, 0x00, 0x00);
    set_tile(offset_y + puzzlerows * 2 + 1, offset_x + puzzlecols * 2 - 2, 
             0x54, 0x00, 0x00);
    set_tile(offset_y + puzzlerows * 2 + 1, offset_x + puzzlecols * 2 - 1, 
             0x4F, 0x00, 0x00);

    // parse raw data
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
            if((puzzledata[idx] & 0x0F) != 0) {
                continue;
            }

            // only check for right-hand non-zero cells
            if(i >= puzzlerows-2 && j+2 < puzzlecols) {
                if(puzzledata[idx + 1] > 0 && puzzledata[idx + 1] < 0x0A &&
                   puzzledata[idx + 2] > 0 && puzzledata[idx + 2] < 0x0A) {
                    puzzledata[idx] |= TLDT_HCLUE;
                }
                continue;
            }

            // only check for bottom cells
            if(j >= puzzlecols-2) {
                if(puzzledata[idx + puzzlecols] > 0 && puzzledata[idx + puzzlecols] < 0x0A &&
                   puzzledata[idx + 2*puzzlecols] > 0 && puzzledata[idx + 2*puzzlecols] < 0x0A) {
                    puzzledata[idx] |= TLDT_VCLUE;
                }
                continue;
            }

            // remaining cells
            if(puzzledata[idx + 1] > 0 && puzzledata[idx + 1] < 0x0A &&
               puzzledata[idx + 2] > 0 && puzzledata[idx + 2] < 0x0A) {
                puzzledata[idx] |= TLDT_HCLUE;
            }

            if(puzzledata[idx + puzzlecols] > 0 && puzzledata[idx + puzzlecols] < 0x0A &&
               puzzledata[idx + 2*puzzlecols] > 0 && puzzledata[idx + 2*puzzlecols] < 0x0A) {
                puzzledata[idx] |= TLDT_VCLUE;
            }
        }
    }

    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {
            idx = i * puzzlecols + j;
            c = puzzledata[idx];

            if(c == 0) {
                set_tile(offset_y + i*2, offset_x + j*2, TILE_BLOCKED, 0x00, 0);
                set_tile(offset_y + i*2+1, offset_x + j*2, TILE_BLOCKED, (1 << 3), 0);
                set_tile(offset_y + i*2, offset_x + j*2+1, TILE_BLOCKED, (1 << 2), 0);
                set_tile(offset_y + i*2+1, offset_x + j*2+1, TILE_BLOCKED, (1 << 2) | (1 << 3), 0);
            } else if(c > 0 && c < 0x0A) { // numeric values
                set_tile(offset_y + i*2, offset_x + j*2, TILE_EMPTY, 0x00, 0);
                set_tile(offset_y + i*2+1, offset_x + j*2, TILE_EMPTY, (1 << 3), 0);
                set_tile(offset_y + i*2, offset_x + j*2+1, TILE_EMPTY, (1 << 2), 0);
                set_tile(offset_y + i*2+1, offset_x + j*2+1, TILE_EMPTY, (1 << 2) | (1 << 3), 0);
            } else {
                set_tile(offset_y + i*2, offset_x + j*2, TILE_CLUE1, 0x00, 0);
                set_tile(offset_y + i*2+1, offset_x + j*2, TILE_CLUE2, 0x00, 0);
                set_tile(offset_y + i*2, offset_x + j*2+1, TILE_CLUE2, (1 << 2) | (1 << 3), 0);
                set_tile(offset_y + i*2+1, offset_x + j*2+1, TILE_CLUE1, (1 << 2) | (1 << 3), 0);
            }

            if((c & 0xF) == 0) {
                puzzledata[idx] |= TLDT_LOCKED;
            } else {
                tiles_incorrect++;
            }
        }
    }

    puzzle_generate_clues();
    puzzle_set_revealed_cells();
}

/**
 * @brief Show the finalized solution
 * 
 */
void show_solution() {
    uint8_t c = 0;
    uint8_t i,j,idx;

    // loop over tiles and place answer tiles
    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {

            // generate right-clue
            idx = i * puzzlecols + j;
            if((puzzledata[idx] & TLDT_LOCKED) == 0) {
                c = puzzledata[idx] & 0x0F;
                set_solution_tile(i, j, c);
            }
        }
    }
}

/**
 * @brief Place a puzzle tile at location
 * 
 * @param y         y-position in puzzle
 * @param x         x-position in puzzle
 * @param tile      tile_id
 */
void set_puzzle_tile(uint8_t y, uint8_t x, uint8_t tile) {
    set_tile(offset_y + y*2, offset_x + x*2, tile, 0x00, 0);
    set_tile(offset_y + y*2+1, offset_x + x*2, tile, (1 << 3), 0);
    set_tile(offset_y + y*2, offset_x + x*2+1, tile, (1 << 2), 0);
    set_tile(offset_y + y*2+1, offset_x + x*2+1, tile, (1 << 2) | (1 << 3), 0);
}

/**
 * @brief Place the solution value at location
 * 
 * @param y             y-position in puzzle
 * @param x             x-position in puzzle
 * @param tile_value    tile_value
 */
void set_solution_tile(uint8_t y, uint8_t x, uint8_t tile_value) {
    if(tile_value < 8) {
        tile_value = tile_value * 2 + 0x20;
    } else {
        tile_value = (tile_value - 8) * 2 + 0x40;
    }
    set_tile(offset_y + y*2, offset_x + x*2, tile_value, 0x00, 1);
    set_tile(offset_y + y*2, offset_x + x*2+1, tile_value + 1, 0x00, 1);
    set_tile(offset_y + y*2+1, offset_x + x*2, tile_value + 0x10, 0x00, 1);
    set_tile(offset_y + y*2+1, offset_x + x*2+1, tile_value + 0x11, 0x00, 1);
}

/**
 * @brief Handle mouse operation
 * 
 */
void puzzle_handle_mouse() {
    static uint8_t mouse_buttons = 0x00;
    uint16_t *mouse_x = (uint16_t *)0x2;
    uint16_t *mouse_y = (uint16_t *)0x4;
    uint8_t idx = 0;

    // read mouse
    asm("ldx #2");
    asm("jsr $FF6B");
    asm("sta %v", mouse_buttons);

    // get board position from mouse position
    ccurx = ((*mouse_x >> 4) - offset_x) >> 1;
    ccury = ((*mouse_y >> 4) - offset_y) >> 1;

    // release highlight
    if(ccurx != ocurx || ccury != ocury) {
        idx = ocury * puzzlecols + ocurx;
        if((puzzledata[idx] & TLDT_LOCKED) == 0) {
            set_puzzle_tile(ocury, ocurx, TILE_EMPTY);
        }
    }

    // place highlight
    if(ccurx >= 0 && ccurx < puzzlecols && ccury >=0 && ccury < puzzlerows) {
        idx = ccury * puzzlecols + ccurx;
        if(puzzledata[idx] & (TLDT_LOCKED | TLDT_REVEALED)) {
            return;
        } else {
            set_puzzle_tile(ccury, ccurx, TILE_HIGHLIGHT);
            ocurx = ccurx;
            ocury = ccury;
        }
    }
}

/**
 * @brief Puzzle handle keyboard interaction
 * 
 */
uint8_t puzzle_handle_keyboard() {
    static uint8_t keycode;
    uint8_t idx;
    char buf[5];

    // grab keycode
    asm("jsr $FFE4");
    asm("sta %v", keycode);

    if(keycode >= 49 && keycode <= 58) { // value between 0-9        
        idx = ccury * puzzlecols + ccurx;

        if(puzzledata[idx] & (TLDT_LOCKED | TLDT_REVEALED)) {
            return 0;
        } else {
            keycode = (keycode - 48) & 0xF;

            if((userdata[idx] & 0xF) != (puzzledata[idx] & 0xF) &&
               (puzzledata[idx] & 0xF) == keycode) {
                tiles_incorrect--;
            }

            if((userdata[idx] & 0xF) == (puzzledata[idx] & 0xF) &&
               (puzzledata[idx] & 0xF) != keycode) {
                tiles_incorrect++;
            }

            userdata[idx] = keycode;
            set_solution_tile(ccury, ccurx, userdata[idx]);
            userdata[idx] |= TLDT_WRITTEN;

            sprintf(buf, "%04X", tiles_incorrect);
            write_debug(buf);

            if(tiles_incorrect == 0) {
                swap_color_font_tiles(0x10, 0x60);
            }
        }

        return 0;
    } else if(keycode == 0x1B) {
        return 1;
    }

    return 0;
}

/**
 * @brief Auxiliary function to generate puzzle clues
 * 
 */
void puzzle_generate_clues() {
    uint8_t c,i,j,idx,ctr;

    ctr = 0x60;
    // loop over tiles and generate clues
    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {

            // generate right-clue
            idx = i * puzzlecols + j;
            if(puzzledata[idx] & TLDT_HCLUE) {
                c = 0;
                idx++;
                while((puzzledata[idx] & 0x0F) > 0 && idx < puzzlecells) {
                    c += puzzledata[idx];
                    idx++;
                }
                build_clue_tile_right(ctr, c);
                set_tile(offset_y + i*2, offset_x + j*2+1, ctr, 0x00, 0);
                ctr++;
            }

            // generate down clue
            idx = i * puzzlecols + j;
            if(puzzledata[idx] & TLDT_VCLUE) {
                c = 0;
                idx += puzzlecols;
                while((puzzledata[idx] & 0x0F) > 0 && idx < (puzzlecells)) {
                    c += puzzledata[idx];
                    idx += puzzlecols;
                }
                build_clue_tile_down(ctr, c);
                set_tile(offset_y + i*2+1, offset_x + j*2, ctr, 0x00, 0);
                ctr++;
            }
        }
    }
}

void puzzle_set_revealed_cells() {
    uint8_t nrcells, nrknowns;
    uint8_t c,i,j,idx;
    uint8_t* v;

    // set ram bank to load puzzle data into
    asm("lda %v", rambank_puzzle);
    asm("sta 0");

    // here, the pointer is now set to the number of 'knowns'
    // reset the pointer to the knowns information in the puzzledata and
    // capture all knowns
    nrcells = puzzlecols * puzzlerows;
    v = (uint8_t*)(0xA000 + data_offset) + (nrcells / 2 + nrcells % 2 + 1);
    nrknowns = *v++;
    // loop over number of knowns and set puzzledata to known data
    for(i=0; i<nrknowns; i++) {
        j = (*v >> 4) & 0x0F;       // row index
        c= *v & 0x0F;               // column index
        idx = j * puzzlecols + c;   // array index

        puzzledata[idx] |= TLDT_REVEALED;
        userdata[idx] = puzzledata[idx] & 0x0F;
        userdata[idx] |= TLDT_WRITTEN;
        set_solution_tile(j, c, puzzledata[idx] & 0x0F);
        set_puzzle_tile(j, c, TILE_REVEALED);
        v++;
    }
    tiles_incorrect -= nrknowns;

    // swap back to default ram bank
    asm("lda 0");
    asm("sta 0");
}