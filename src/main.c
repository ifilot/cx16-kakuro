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

#include <cx16.h>
#include <stdint.h>

#include "video.h"
#include "puzzle.h"
#include "tile.h"
#include "mouse.h"
#include "menu.h"
#include "sound.h"

uint16_t puzzle_filesize; // file size of the puzzle files

void load_puzzles(); // forward declaration
void save_puzzles(); // forward declaration

void main() {
    // load assets into memory
    init_screen();

    // enable mouse
    init_mouse();
    set_mouse_pointer(0x30);

    // load sound engine
    init_sound();
    start_bgmusic();

    // load into VERA
    load_tiles("mtiles.dat", TILEBASE_MENU);
    load_tiles("tiles.dat", TILEBASE_GAME);
    load_tiles("font16.dat", TILEBASE_FONT16);
    load_tiles("font8.dat", TILEBASE_FONT8);

    // load assets
    load_small_digits();

    while(1) {
        clear_screen();

        // load puzzles into memory
        load_puzzles();

        set_tilebase_layer0(TILEBASE_MENU);
        build_menu();
        while(1) {
            if(menu_handle_mouse() == 1) {
                break;
            }
            sound_fill_buffers();
        }

        clear_screen();
        set_tilebase_layer0(TILEBASE_GAME);
        build_puzzle(current_puzzle_id);
        gamestate = GAME_PLAY;
        while(!(gamestate & GAME_QUIT)) {
            puzzle_handle_mouse();
            puzzle_handle_keyboard();
            show_game_time();

            if(gamestate & GAME_COMPLETE) {
                break;
            }
            sound_fill_buffers();
        }
        save_puzzles();
    }
}

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