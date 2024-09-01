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
int8_t mp_ocurx = 0;
int8_t mp_ocury = 0;
uint16_t tiles_incorrect = 0;
uint16_t data_offset = 0;
uint8_t current_puzzle_id = 0;
uint8_t gamestate = 0;
clock_t game_start_time = 0;
clock_t prevtotal;
char game_timebuffer[10];

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
    asm("lda #%b", RAMBANK_PUZZLE);
    asm("sta 0");

    data_offset = *(uint16_t*)(BANKED_RAM + (puzzle_id+1) * 2);
    v = (uint8_t*)(BANKED_RAM + data_offset);

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

    // build icons
    set_tile(1, 38, 0x08, 0x00, LAYER0);
    set_tile(2, 38, 0x09, 0x00, LAYER0);

    // set puzzle status
    idx = retrieve_puzzle_status(current_puzzle_id + 1);
    idx |= STATUS_OPENED;
    set_puzzle_status(current_puzzle_id+1, idx, 0, 0, 0);

    // keep track of time
    game_start_time = clock();
    prevtotal = -1;
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
                set_solution_tile(i, j, c, 0x12);
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
void set_solution_tile(uint8_t y, uint8_t x, uint8_t tile_value, uint8_t col) {
    if(tile_value < 8) {
        tile_value = tile_value * 2;
    } else {
        tile_value = (tile_value - 8) * 2 + 0x80;
    }
    set_tile(offset_y + y*2, offset_x + x*2, tile_value, col, LAYER1);
    set_tile(offset_y + y*2, offset_x + x*2+1, tile_value + 1, col, LAYER1);
    set_tile(offset_y + y*2+1, offset_x + x*2, tile_value + 0x10, col, LAYER1);
    set_tile(offset_y + y*2+1, offset_x + x*2+1, tile_value + 0x11, col, LAYER1);
}

/**
 * @brief Handle mouse operation
 * 
 */
void puzzle_handle_mouse() {
    static uint8_t mouse_buttons = 0x00;
    uint8_t tpx = 0;
    uint8_t tpy = 0;
    uint16_t *mouse_x = (uint16_t *)0x2;
    uint16_t *mouse_y = (uint16_t *)0x4;
    uint16_t idx = 0;

    // read mouse
    asm("ldx #2");
    asm("jsr $FF6B");
    asm("sta %v", mouse_buttons);

    // get board position from mouse position
    ccurx = ((*mouse_x >> 4) - offset_x) >> 1;
    ccury = ((*mouse_y >> 4) - offset_y) >> 1;
    tpx = *mouse_x >> 4;
    tpy = *mouse_y >> 4;

    if(tpy == 1 && tpx == 38) {
        set_tile(1, 38, 0x0A, 0x00, LAYER0);
        if(mouse_buttons & 1) {
            while(mouse_buttons != 0x00) {
            asm("ldx #2");
            asm("jsr $FF6B");
            asm("sta %v", mouse_buttons);
            }
            gamestate |= GAME_QUIT;
        }
    } else {
        set_tile(1, 38, 0x08, 0x00, LAYER0);
    }

    if(tpy == 2 && tpx == 38) {
        set_tile(2, 38, 0x0B, 0x00, LAYER0);
        if(mouse_buttons & 1) {
            while(mouse_buttons != 0x00) {
                asm("ldx #2");
                asm("jsr $FF6B");
                asm("sta %v", mouse_buttons);
            }
            gamestate ^= GAME_VERIFY;
            puzzle_color_numbers();
        }
    } else {
        set_tile(2, 38, 0x09, 0x00, LAYER0);
    }

    // release highlight
    if(ccurx != mp_ocurx || ccury != mp_ocury) {
        idx = mp_ocury * puzzlecols + mp_ocurx;
        if((puzzledata[idx] & TLDT_LOCKED) == 0) {
            set_puzzle_tile(mp_ocury, mp_ocurx, TILE_EMPTY);
        }
    }

    // place highlight
    if(ccurx >= 0 && ccurx < puzzlecols && ccury >=0 && ccury < puzzlerows) {
        idx = ccury * puzzlecols + ccurx;
        if((puzzledata[idx] & (TLDT_LOCKED | TLDT_REVEALED)) == 0) {
            set_puzzle_tile(ccury, ccurx, TILE_HIGHLIGHT);
            mp_ocurx = ccurx;
            mp_ocury = ccury;
        }
    }
}

/**
 * @brief Puzzle handle keyboard interaction
 * 
 */
void puzzle_handle_keyboard() {
    static uint8_t keycode;
    uint8_t idx;

    // grab keycode
    asm("jsr $FFE4");
    asm("sta %v", keycode);

    if(keycode >= '1' && keycode <= '9') { // value between 0-9        
        idx = ccury * puzzlecols + ccurx;

        if(puzzledata[idx] & (TLDT_LOCKED | TLDT_REVEALED)) {
            return;
        } else {
            keycode = (keycode - '0') & 0xF;

            if((userdata[idx] & 0xF) != (puzzledata[idx] & 0xF) &&
               (puzzledata[idx] & 0xF) == keycode) {
                tiles_incorrect--;
            }

            if((userdata[idx] & 0xF) == (puzzledata[idx] & 0xF) &&
               (puzzledata[idx] & 0xF) != keycode) {
                tiles_incorrect++;
            }

            userdata[idx] = keycode - '0';
            if(gamestate & GAME_VERIFY) {
                if((puzzledata[idx] & 0x0F) == (userdata[idx] & 0x0F)) {
                    set_solution_tile(ccury, ccurx, userdata[idx] & 0x0F, 0x5F);
                } else {
                    set_solution_tile(ccury, ccurx, userdata[idx] & 0x0F, 0x36);
                }
            } else {
                set_solution_tile(ccury, ccurx, userdata[idx] & 0x0F, 0x12);
            }
            userdata[idx] |= TLDT_WRITTEN;

            if(tiles_incorrect == 0) {
                puzzle_complete();
            }
        }
    } else if(keycode == KEYCODE_ESCAPE) {
        printtext("Are you sure you want to quit?", 2, 2, 0x12);
        printtext("YES (Y) / NO (N)", 3, 2, 0x12);
        while(keycode != 'Y' && keycode != 'N') {
            asm("jsr $FFE4");
            asm("sta %v", keycode);
        }
        if(keycode == 'Y') {
            gamestate |= GAME_QUIT;
        } else {
            printspaces(30, 2, 2);
            printspaces(16, 3, 2);
        }
    }
}

/**
 * @brief Auxiliary function to generate puzzle clues
 * 
 */
void puzzle_generate_clues() {
    uint8_t c,i,j,idx,vrampos = 0;

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
                build_clue_tile_right(vrampos, c);
                set_tile(offset_y + i*2, offset_x + j*2+1, 0x20+vrampos, 0x00, 0);
                vrampos++;
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
                build_clue_tile_down(vrampos, c);
                set_tile(offset_y + i*2+1, offset_x + j*2, 0x20+vrampos, 0x00, 0);
                vrampos++;
            }
        }
    }
}

void puzzle_set_revealed_cells() {
    uint8_t nrcells, nrknowns;
    uint8_t c,i,j,idx;
    uint8_t* v;

    // set ram bank to load puzzle data into
    asm("lda #%b", RAMBANK_PUZZLE);
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
        set_solution_tile(j, c, puzzledata[idx] & 0x0F, 0x12);
        set_puzzle_tile(j, c, TILE_REVEALED);
        v++;
    }
    tiles_incorrect -= nrknowns;

    // swap back to default ram bank
    asm("lda 0");
    asm("sta 0");
}

/**
 * @brief Color numbers in cells based on whether auto-verification has been
 *        turned on or not.
 */
void puzzle_color_numbers() {
    uint8_t i,j;
    uint8_t idx;
    // loop over tiles and generate clues
    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {
            idx = i * puzzlecols + j;
            if(userdata[idx] & TLDT_WRITTEN ) {
                if(gamestate & GAME_VERIFY) {
                    if((puzzledata[idx] & 0x0F) == (userdata[idx] & 0x0F)) {
                        set_solution_tile(i, j, userdata[idx] & 0x0F, 0x5F);
                    } else {
                        set_solution_tile(i, j, userdata[idx] & 0x0F, 0x36);
                    }
                } else {
                    set_solution_tile(i, j, userdata[idx] & 0x0F, 0x12);
                }
            }
        }
    }
}

/**
 * @brief Show game time
 * 
 */
void show_game_time() {
    calculate_game_time();
    printtext(game_timebuffer, 29, 32, 0x15);
}

/**
 * @brief Retrieve puzzle status based on puzzle id
 * 
 * @param puzzle_id puzzle_id; start counting from 1
 * @return uint8_t 
 */
uint8_t retrieve_puzzle_status(uint8_t puzzle_id) {
    uint8_t *v;
    uint8_t ret;

    // set ram bank to load puzzle data into
    asm("lda #%b", RAMBANK_PUZZLE);
    asm("sta 0");

    v = get_puzzle_pointer(puzzle_id);

    // grab first byte
    ret = *v;

    asm("lda 0");
    asm("sta 0");

    return ret;
}

/**
 * @brief Set the puzzle status in memory
 */
void set_puzzle_status(uint8_t puzzle_id, uint8_t status, uint8_t hours,
                       uint8_t minutes, uint8_t seconds) {
    uint8_t *v;

    // set ram bank to load puzzle data into
    asm("lda #%b", RAMBANK_PUZZLE);
    asm("sta 0");

    v = get_puzzle_pointer(puzzle_id);

    // increment pointer by number of knowns multiplied by 2
    *v++ = status;
    *v++ = hours;
    *v++ = minutes;
    *v = seconds;

    asm("lda 0");
    asm("sta 0");
}

/**
 * @brief Get the pointer to the puzzle status data given puzzle_id
 * 
 * MAY ONLY BE USED IN FUNCTIONS THAT HAVE ALREADY DONE THE BANK SWITCHING
 * 
 * @param puzzle_id 
 * @return uint8_t* 
 */
uint8_t* get_puzzle_pointer(uint8_t puzzle_id) {
    uint8_t nrcells = 0;
    uint8_t *v;

    // pointer is here to begin of puzzle data
    v = (uint8_t*)(0xA000 + *(uint16_t*)(0xA000 + (puzzle_id * 2)));

    // increment pointer by number of bytes of cell data to consume
    nrcells = ((*v >> 4) & 0x0F) * (*v & 0x0F);
    v += (nrcells >> 1) + (nrcells & 1) + 1;

    // increment pointer by number of knowns
    v += *(v++);

    return v;
}

/**
 * @brief Calculate the number of incorrect tiles
 * 
 * @return uint8_t* 
 */
uint8_t get_nr_incorrect_tiles() {
    uint8_t i,j;
    uint8_t idx;
    uint8_t incorrect = 0;

    // loop over tiles and generate clues
    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {
            idx = i * puzzlecols + j;
            if((puzzledata[idx] & 0x0F) != (userdata[idx] & 0x0F)) {
                incorrect++;
            }
        }
    }

    return incorrect;
}

/**
 * @brief Routine to invoke when the puzzle has been completed by the user
 * 
 */
void puzzle_complete() {
    uint8_t i,j,idx;

    // first, color all tiles green
    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {
            idx = i * puzzlecols + j;
            if((puzzledata[idx] & 0x0F) != 0) {
                set_solution_tile(i, j, userdata[idx] & 0x0F, 0x5F);
            }
        }
    }

    // next, provide a message to the user
    printtext("Congratulations!!", 2, 2, 0x10);
    printtext("You finished the puzzle!", 3, 2, 0x15);
    printtext("Your time is: ", 4, 2, 0x15);
    calculate_game_time();
    printtext(game_timebuffer, 4, 16, 0x10);
    printtext("Press ENTER to return to menu.", 5, 2, 0x15);
    wait_for_key(KEYCODE_RETURN);

    // write game state
    store_puzzle_state();

    // update game state
    gamestate |= GAME_COMPLETE;
}

/**
 * @brief Auxiliary function that computes the game time and stores it in buffer
 * 
 * @param buf pointer to buffer
 */
void calculate_game_time() {
    uint8_t hours, minutes, seconds;
    clock_t end = clock();
    clock_t total = (end - game_start_time) / CLOCKS_PER_SEC;
    
    // early exit to save upon some clock cycles
    if(total == prevtotal) {
        return;
    } else {
        prevtotal = total;
    }

    hours = total / 3600;
    total -= hours * 3600;
    minutes = total / 60;
    seconds = total % 60;

    sprintf(game_timebuffer, "%02U:%02U:%02U", hours, minutes, seconds);
}

/**
 * @brief Store the puzzle state at the end of the game
 * 
 */
void store_puzzle_state() {
    uint8_t status;
    uint8_t hours, minutes, seconds;
    clock_t end = clock();
    clock_t total = (end - game_start_time) / CLOCKS_PER_SEC;
    
    // early exit to save upon some clock cycles
    if(total == prevtotal) {
        return;
    } else {
        prevtotal = total;
    }

    hours = total / 3600;
    total -= hours * 3600;
    minutes = total / 60;
    seconds = total % 60;

    status = retrieve_puzzle_status(current_puzzle_id+1);
    status |= STATUS_SOLVED;
    set_puzzle_status(current_puzzle_id+1, status, hours, minutes, seconds);
}

/**
 * @brief Wait for key to be pressed
 * 
 * @param key 
 */
void wait_for_key(uint8_t key) {
    static uint8_t keycode = 0;

    // consume any previous characters
    while(keycode == key) {
        asm("jsr $FFE4");
        asm("sta %v", keycode);
    }

    // wait until a new character is seen
    while(keycode != key) {
        asm("jsr $FFE4");
        asm("sta %v", keycode);
    }
}