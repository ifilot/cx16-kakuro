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
    load_tiles("mtiles.dat");
    load_small_digits();
    clear_screen();

    // build the menu / puzzle
    //build_puzzle();
    build_menu();

    // enable mouse
    init_mouse();

    while(1) {
        menu_handle_mouse();
    }

    // while(1) {
    //     puzzle_handle_mouse();
    //     puzzle_handle_keyboard();
    // }
}