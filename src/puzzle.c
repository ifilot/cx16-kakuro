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
#include "playfield.h"

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
uint16_t puzzle_filesize;       // file size of the puzzle files
extern uint8_t pco = 2;         // puzzle color offset

/**
 * @brief Clears the screen
 * 
 * Set background tiles everywhere and sets foreground to transparent
 * 
 */
void puzzle_clear_screen() {
    fill_layer(TILE_BACKGROUND + (pco << 3), LAYER0, PALETTEBYTE, 64, 64);
    fill_layer(0x20, LAYER1, 0x00, 64, 64);
}

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
    free(puzzledata);
    free(userdata);
    puzzledata = (uint8_t*)malloc(puzzlecells);
    userdata = (uint8_t*)calloc(puzzlecells, 1);
    v++;

    offset_x = 14 - puzzlecols;
    offset_y = 16 - puzzlerows;
    ccurx = ccury = mp_ocurx = mp_ocury = -1;

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

            if(j+2<puzzlecols && (puzzledata[idx+1]&15) && (puzzledata[idx+2]&15))
                puzzledata[idx] |= TLDT_HCLUE;
            if(i+2<puzzlerows && (puzzledata[idx+puzzlecols]&15) &&
               (puzzledata[idx+2*puzzlecols]&15))
                puzzledata[idx] |= TLDT_VCLUE;
        }
    }

    playfield_enter();
    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {
            idx = i * puzzlecols + j;
            if((puzzledata[idx] & 15) == 0)puzzledata[idx] |= TLDT_LOCKED;
            else tiles_incorrect++;
            playfield_cell(i,j);
        }
    }
    puzzle_generate_clues();
    puzzle_set_revealed_cells();

    // set puzzle status
    idx = retrieve_puzzle_status(current_puzzle_id + 1);
    idx |= STATUS_OPENED;
    set_puzzle_status(current_puzzle_id+1, idx, 0, 0, 0);

    // keep track of time
    game_start_time = clock();
    prevtotal = -1;
    show_game_time();
    playfield_ready();
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
                userdata[idx]=c|TLDT_WRITTEN;
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
void set_puzzle_tile(uint8_t y,uint8_t x,uint8_t tile) {
    (void)tile;
    playfield_cell(y,x);
}

void set_solution_tile(uint8_t y,uint8_t x,uint8_t tile_value,uint8_t col) {
    (void)tile_value;
    (void)col;
    playfield_cell(y,x);
}

/**
 * @brief Handle mouse operation
 * 
 */
void puzzle_handle_mouse() {
    static uint8_t buttons=0,previous=0;
    static int8_t pressed=-1;
    uint16_t* mx=(uint16_t*)2;
    uint16_t* my=(uint16_t*)4;
    int8_t target=-1;
    uint8_t idx;
    asm("ldx #2");asm("jsr $FF6B");asm("sta %v",buttons);
    if(*mx>=436 && *mx<600) {
        if(*my>=288 && *my<320)target=1;
    }
    playfield_controls(target);
    ccurx=ccury=-1;
    if(*mx>=offset_x*16 && *mx<(offset_x+puzzlecols*2)*16 &&
       *my>=offset_y*16 && *my<(offset_y+puzzlerows*2)*16) {
        ccurx=(*mx-offset_x*16)>>5;
        ccury=(*my-offset_y*16)>>5;
        idx=ccury*puzzlecols+ccurx;
        if(puzzledata[idx]&(TLDT_LOCKED|TLDT_REVEALED))ccurx=ccury=-1;
    }
    if(ccurx!=mp_ocurx || ccury!=mp_ocury) {
        if(mp_ocurx>=0 && mp_ocury>=0)playfield_cell(mp_ocury,mp_ocurx);
        if(ccurx>=0 && ccury>=0)playfield_cell(ccury,ccurx);
        mp_ocurx=ccurx;mp_ocury=ccury;
    }
    if((buttons&1) && !(previous&1))pressed=target;
    if(!(buttons&1) && (previous&1)) {
        if(pressed==target && target==1) {
            gamestate^=GAME_VERIFY;
            play_sfx(gamestate&GAME_VERIFY ? SFX_VERIFY_ON : SFX_VERIFY_OFF);
            puzzle_color_numbers();
            playfield_controls(target);
        }
        pressed=-1;
    }
    previous=buttons;
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

    if(keycode == 0x14 || keycode == 0x08 || keycode == 0x7F || keycode == '0') {
        if(ccurx<0 || ccury<0)return;
        idx=ccury*puzzlecols+ccurx;
        if(userdata[idx] && (userdata[idx]&15)==(puzzledata[idx]&15))tiles_incorrect++;
        if(userdata[idx])play_sfx(SFX_BACK);
        userdata[idx]=0;
        playfield_cell(ccury,ccurx);
    } else if(keycode >= '1' && keycode <= '9') {
        if(ccurx<0 || ccury<0)return;
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

            userdata[idx] = keycode | TLDT_WRITTEN;
            playfield_cell(ccury,ccurx);
            if((gamestate&GAME_VERIFY) && keycode!=(puzzledata[idx]&15))play_sfx(SFX_WRONG);
            else sound_digit(keycode);

            if(tiles_incorrect == 0) {
                puzzle_complete();
            }
        }
    } else if(keycode == KEYCODE_ESCAPE) {
        puzzle_quit();
    }
}

/**
 * @brief Auxiliary function to generate puzzle clues
 * 
 */
void puzzle_generate_clues() {
    uint8_t c,i,j,idx;

    // loop over tiles and generate clues
    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {

            // generate right-clue
            idx = i * puzzlecols + j;
            if(puzzledata[idx] & TLDT_HCLUE) {
                c = 0;
                idx++;
                while(idx < (i+1)*puzzlecols && (puzzledata[idx] & 0x0F) > 0) {
                    c += puzzledata[idx]&15;
                    idx++;
                }
                playfield_clue(i,j,c,0);
            }

            // generate down clue
            idx = i * puzzlecols + j;
            if(puzzledata[idx] & TLDT_VCLUE) {
                c = 0;
                idx += puzzlecols;
                while(idx < puzzlecells && (puzzledata[idx] & 0x0F) > 0) {
                    c += puzzledata[idx]&15;
                    idx += puzzlecols;
                }
                playfield_clue(i,j,c,1);
            }

            // this can take quite some time, avoid the sound buffer from emptying
            sound_fill_buffers();
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
    clock_t previous_time=prevtotal;
    calculate_game_time();
    if(prevtotal!=previous_time)playfield_clock(game_timebuffer);
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

    v = get_puzzle_status_pointer(puzzle_id);

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

    v = get_puzzle_status_pointer(puzzle_id);

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
uint8_t* get_puzzle_status_pointer(uint8_t puzzle_id) {
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
    play_sfx(SFX_SOLVED);
    gamestate |= GAME_VERIFY;

    // Mark all completed entries with the journal correctness indicator
    for(i=0; i<puzzlerows; i++) {
        for(j=0; j<puzzlecols; j++) {
            idx = i * puzzlecols + j;
            if((puzzledata[idx] & 0x0F) != 0) {
                set_solution_tile(i, j, userdata[idx] & 0x0F, 0x5F);
            }
        }
    }

    // next, provide a message to the user
    playfield_window(11,5,4,30);
    playfield_text("Congratulations!!", 11, 5);
    playfield_text("You finished the puzzle!", 12, 5);
    playfield_text("Your time is: ", 13, 5);
    calculate_game_time();
    playfield_text(game_timebuffer, 13, 5+14);
    playfield_text("Press ENTER to return to menu.", 14, 5);
    wait_for_key(KEYCODE_RETURN);

    // write game state
    store_puzzle_state();

    // update game state
    gamestate |= GAME_COMPLETE;
}

/**
 * @brief Routine to invoke when user wants to quit the puzzle
 * 
 */
void puzzle_quit() {
    static uint8_t keycode = 0xFF,buttons;
    uint8_t previous=0;
    int8_t hover=-1,next,pressed=-1;
    uint16_t x,y;
    play_sfx(SFX_DIALOG);
    playfield_save();
    playfield_quit_modal();
    // consume previous keycode
    while(keycode != 0) {
        asm("jsr $FFE4");
        asm("sta %v", keycode);
    }

    while(keycode != 'Y' && keycode != 'N') {
        asm("jsr $FFE4");
        asm("sta %v", keycode);
        if(keycode==KEYCODE_ESCAPE)keycode='N';
        asm("ldx #2");asm("jsr $FF6B");asm("sta %v",buttons);
        x=*(uint16_t*)2;y=*(uint16_t*)4;next=-1;
        if(y>=256 && y<276) {
            if(x>=168 && x<296)next=0;
            if(x>=344 && x<472)next=1;
        }
        if(next!=hover) {hover=next;playfield_quit_hover(hover);}
        if((buttons&1) && !previous)pressed=hover;
        if(!(buttons&1) && previous && pressed==hover && hover>=0)keycode=hover ? 'N' : 'Y';
        previous=buttons&1;
        sound_fill_buffers();
    }
    if(keycode == 'Y') {
        play_sfx(SFX_BACK);
        gamestate |= GAME_QUIT;
    } else {
        play_sfx(SFX_BACK);
        playfield_restore();
    }
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
        sound_fill_buffers();
    }
}

/**
 * @brief Build window
 * 
 * @param x x-position
 * @param y y-position
 * @param w width
 * @param h height
 */
void build_window(uint8_t y, uint8_t x, uint8_t h, uint8_t w) {
    uint8_t i,j;

    // top and bottom borders
    set_tile(y-1, x-1, WINDOW_LEFT_TOP, 0x00, LAYER0);          // left-top
    set_tile(y+h, x-1, WINDOW_LEFT_BOTTOM, 0x00, LAYER0);       // left-bottom
    for(i=0; i<w; i++) {
        set_tile(y-1, x+i, WINDOW_TOP, 0x00, LAYER0);           // top layer
        set_tile(y+h, x+i, WINDOW_BOTTOM, 0x00, LAYER0);        // bottom layer
    }
    set_tile(y-1, x+w, WINDOW_RIGHT_TOP, MIRROR_X, LAYER0);     // right-top
    set_tile(y+h, x+w, WINDOW_RIGHT_BOTTOM, MIRROR_X, LAYER0);  // right-bottom

    // left and right borders
    for(i=0; i<h; i++) {
        set_tile(y+i, x-1, WINDOW_HOR_EDGE, 0x00, LAYER0);
        set_tile(y+i, x+w, WINDOW_HOR_EDGE, MIRROR_X, LAYER0);
    }

    // core
    for(i=0; i<h; i++) {
        for(j=0; j<w; j++) {
            set_tile(y+i, x+j, WINDOW_CORE, MIRROR_X, LAYER0);
        }
    }

    // remove any text tiles
    for(i=0; i<h+2; i++) {
        for(j=0; j<w+2; j++) {
            set_tile(y+i-1, x+j-1, 0x00, 0x00, LAYER1);
        }
    }
}

/**
 * @brief Print the border around the clock
 * 
 * @param y y-position
 * @param x x-position
 */
void print_clock_border(uint8_t y, uint8_t x) {
    uint8_t i;
    set_tile(y-1, x-1, CLOCK_COR + (pco * 3), 0x00, LAYER0);         // top left
    set_tile(y, x-1, CLOCK_VER + (pco * 3), 0x00, LAYER0);           // mid left
    set_tile(y+1, x-1, CLOCK_COR + (pco * 3), MIRROR_Y, LAYER0);     // bottom left
    for(i=0; i<8; i++) {
        set_tile(y-1, x+i, CLOCK_HOR + (pco * 3), 0x00, LAYER0);
        set_tile(y+1, x+i, CLOCK_HOR + (pco * 3), MIRROR_Y, LAYER0);
    }
    set_tile(y-1, x+8, CLOCK_COR + (pco * 3), MIRROR_X, LAYER0);     // top right
    set_tile(y, x+8, CLOCK_VER + (pco * 3), MIRROR_X, LAYER0);       // mid right
    set_tile(y+1, x+8, CLOCK_COR + (pco * 3), MIRROR_XY, LAYER0);    // bottom right
}

void print_clock(const char* s, uint8_t y, uint8_t x) {
    while(*s != 0) {
        if(*s == ':') {
            set_tile(y, x, 0x4A, 0x00, LAYER0);
        } else {
            set_tile(y, x, *s - '0' + 0x40, 0x00, LAYER0);
        }
        x++;
        s++;
    }
}

/**
 * @brief Load puzzles data into memory
 * 
 */
void load_puzzles() {
    asm("lda #%b", RAMBANK_PUZZLE);
    asm("sta 0");

    // load puzzle into memory
    cbm_k_setnam("puzzle.dat");
    cbm_k_setlfs(0, 8, 1);
    puzzle_filesize = cbm_k_load(0, 0) - BANKED_RAM;

    asm("lda 0");
    asm("sta 0");
}

/**
 * @brief Save puzzles data to SD-card
 * 
 */
void save_puzzles() {
    asm("lda #%b", RAMBANK_PUZZLE);
    asm("sta 0");

    // load puzzle into memory
    cbm_k_setnam("@:puzzle.dat"); // force overwrite
    cbm_k_setlfs(1, 8, 2);
    cbm_k_save(BANKED_RAM, BANKED_RAM + puzzle_filesize);

    asm("lda 0");
    asm("sta 0");
}