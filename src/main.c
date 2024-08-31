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

void main() {
    // load assets into memory
    init_screen();

    // enable mouse
    init_mouse();

    // load into VERA
    load_tiles("mtiles.dat", TILEBASE_MENU);
    load_tiles("tiles.dat", TILEBASE_GAME);
    load_tiles("fontmap.dat", TILEBASE_FONT);

    // load assets
    load_small_digits();

    while(1) {
        // clear_screen();
        // set_tilebase_layer0(TILEBASE_MENU);
        // build_menu();
        // while(1) {
        //     if(menu_handle_mouse() == 1) {
        //         break;
        //     }
        // }

        clear_screen();
        set_tilebase_layer0(TILEBASE_GAME);
        build_puzzle(current_puzzle_id);
        while(1) {
            puzzle_handle_mouse();
            if(puzzle_handle_keyboard() == 1) {
                break;
            }
        }
    }
}