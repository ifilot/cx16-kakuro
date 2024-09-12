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

#include "menu.h"

int8_t obtnx = 0; 
int8_t obtny = 0;
uint8_t menu_page = 0;

/**
 * @brief Build the puzzle selection  menu
 * 
 */
void menu_init() {
    uint8_t i,j,puzidx;
    
    // window background
    for(i=2; i<=27; i++) {
        set_tile(i, 1, 0x04, 0x00, LAYER0);
        for(j=2; j<=37; j++) {
            set_tile(i, j, 0x06, 0x00, LAYER0);
        }
        set_tile(i, 38, 0x04, MIRROR_X, LAYER0);
    }

    // window title
    set_tile(1, 1, 0x0B, 0x00, LAYER0);
    for(j=2; j<=37; j++) {
        set_tile(1, j, 0x03, 0x00, LAYER0);
        set_tile(28, j, 0x05, 0x00, LAYER0);
    }
    set_tile(1, 38, 0x02, MIRROR_X, LAYER0);

    if((menu_page+1) < MAX_PAGES) {
        menu_build_tile_forward(0);
    } else {
        menu_build_corner_forward();
    }

    if(menu_page > 0) {
        menu_build_tile_backward(0);
    } else {
        menu_build_corner_backward();
    }

    puzidx = menu_page * 48 + 1;
    for(i=0; i<6; i++) {
        for(j=0; j<8; j++) {
            menu_build_icon(i, j, puzidx++, 0);
        }
    }

    // Kakuro name
    set_tile(2, 34, 0x1A, 0x00, LAYER0);
    set_tile(2, 35, 0x1B, 0x00, LAYER0);
    set_tile(2, 36, 0x2A, 0x00, LAYER0);
    set_tile(2, 37, 0x2B, 0x00, LAYER0);

    // print text
    printtext("Kakuro", 2, 2, 0x12);
    printtext("Help  Options  About", 28, ML_EXPL, 0x14);
    menu_print_page();
}

/**
 * @brief Build a menu page turning tile for the forward direction
 * 
 */
void menu_build_tile_forward(uint8_t col) {
    set_tile(27, 37, 0x08 + col * 0x30, MIRROR_X, LAYER0);
    set_tile(27, 38, 0x07 + col * 0x30, MIRROR_X, LAYER0);
    set_tile(28, 37, 0x0A + col * 0x30, MIRROR_X, LAYER0);
    set_tile(28, 38, 0x09 + col * 0x30, MIRROR_X, LAYER0);
}

/**
 * @brief Build a menu page turning tile for the backward direction
 * 
 */
void menu_build_tile_backward(uint8_t col) {
    set_tile(27, 1, 0x07 + col * 0x30, 0x00, LAYER0);
    set_tile(27, 2, 0x08 + col * 0x30, 0x00, LAYER0);
    set_tile(28, 1, 0x09 + col * 0x30, 0x00, LAYER0);
    set_tile(28, 2, 0x0A + col * 0x30, 0x00, LAYER0);
}

/**
 * @brief Build a menu page turning tile for the forward direction
 * 
 */
void menu_build_corner_forward() {
    set_tile(27, 37, 0x06, MIRROR_X, LAYER0);
    set_tile(27, 38, 0x04, MIRROR_X, LAYER0);
    set_tile(28, 37, 0x05, MIRROR_X, LAYER0);
    set_tile(28, 38, 0x0C, MIRROR_X, LAYER0);
}

/**
 * @brief Build a menu page turning tile for the backward direction
 * 
 */
void menu_build_corner_backward() {
    set_tile(27, 1, 0x04, 0x00, LAYER0);
    set_tile(27, 2, 0x06, 0x00, LAYER0);
    set_tile(28, 1, 0x0C, 0x00, LAYER0);
    set_tile(28, 2, 0x05, 0x00, LAYER0);
}

/**
 * @brief Build a menu page turning tile for the backward direction
 * 
 */
void menu_build_icon(uint8_t y, uint8_t x, uint8_t puzzle_id, uint8_t select) {
    uint8_t i = 0;
    uint8_t status = 0;
    uint8_t col_id = 0;
    uint8_t rows = 0;
    uint8_t diff = 0;

    status = retrieve_puzzle_status(puzzle_id);
    if(select == 1) {
        col_id = 1;
    } else {
        if(status & STATUS_SOLVED) {
            col_id = 2;
        } else if(status & STATUS_OPENED) {
            col_id = 3;
        }
    }

    rows = (status >> 2) & 0x07;
    diff = (status >> 6) & 0x03;

    // show puzzle icon
    set_tile(4*y+4, 4*x+5, 0x48 + col_id, 0x00, LAYER0);
    set_tile(4*y+4, 4*x+6, 0x40 + diff, 0x00, LAYER0);
    set_tile(4*y+5, 4*x+5, 0x50 + rows, 0x00, LAYER0);
    set_tile(4*y+5, 4*x+6, 0x58 + col_id, 0x00, LAYER0);

    col_id = select & 1;

    // set puzzle number
    set_tile(4*y+6, 4*x+5, (puzzle_id / 10) + (col_id + 1) * 0x10, 0x00, LAYER0);
    set_tile(4*y+6, 4*x+6, (puzzle_id % 10) + (col_id + 1) * 0x10, 0x00, LAYER0);
}

/**
 * @brief Handle mouse operation
 * 
 */
uint8_t menu_handle_mouse() {
    static uint8_t mouse_buttons = 0x00;
    uint16_t *mouse_x = (uint16_t *)0x2;
    uint16_t *mouse_y = (uint16_t *)0x4;
    uint8_t scrn_tile_x, scrn_tile_y;
    uint8_t mod_tile_x, mod_tile_y;
    uint8_t pos_x, pos_y;
    uint8_t idx = 0;

    // read mouse
    asm("ldx #2");
    asm("jsr $FF6B");
    asm("sta %v", mouse_buttons);

    // get game button position from mouse position
    scrn_tile_x = (*mouse_x >> 4);  // divide by 16, screen tile position
    scrn_tile_y = (*mouse_y >> 4);
    mod_tile_x = scrn_tile_x % 4;   // get modulus for division by 4
    mod_tile_y = scrn_tile_y % 4;

    // puzzle tiles
    if(mod_tile_y <= 2 && (mod_tile_x == 1 || mod_tile_x == 2)) {
        pos_x = (scrn_tile_x - 5) >> 2;
        pos_y = (scrn_tile_y - 4) >> 2;
        if(pos_y > 5 || pos_x > 7) {
            // do nothing
        } else {
            if(pos_y != obtny || pos_x != obtnx) {
                menu_build_icon(obtny, obtnx, obtny * 8 + obtnx + 1 + menu_page * 48, 0); // release
            }
            menu_build_icon(pos_y, pos_x, pos_y * 8 + pos_x + 1 + menu_page * 48, 1);     // highlight
            obtnx = pos_x;
            obtny = pos_y;

            if(mouse_buttons & 1) {
                while(mouse_buttons != 0x00) {
                    asm("ldx #2");
                    asm("jsr $FF6B");
                    asm("sta %v", mouse_buttons);
                }

                current_puzzle_id = obtny * 8 + obtnx + menu_page * 48;
                gamestate = GAME_PLAY;
                return 1;
            }
        }
    } else {
        menu_build_icon(obtny, obtnx, obtny * 8 + obtnx + 1 + menu_page * 48, 0); // release
    }

    // page turning tiles
    if(scrn_tile_y >= 27 && scrn_tile_y <=28 && 
       scrn_tile_x >= 1 && scrn_tile_x <= 2 &&
       menu_page > 0) {
        menu_build_tile_backward(1);
        if(mouse_buttons & 1) {
            while(mouse_buttons != 0x00) {
                asm("ldx #2");
                asm("jsr $FF6B");
                asm("sta %v", mouse_buttons);
            }
            menu_page--;
            menu_init();
            return 0;
        }
    } else {
        if(menu_page > 0) {
            menu_build_tile_backward(0);
        } else {
            menu_build_corner_backward();
        }
    }

    if(scrn_tile_y >= 27 && scrn_tile_y <=28 && 
       scrn_tile_x >= 37 && scrn_tile_x <= 38 &&
       (menu_page+1) < MAX_PAGES) {
        menu_build_tile_forward(1);
        if(mouse_buttons & 1) {
            while(mouse_buttons != 0x00) {
                asm("ldx #2");
                asm("jsr $FF6B");
                asm("sta %v", mouse_buttons);
            }
            menu_page++;
            menu_init();
            return 0;
        }
    } else {
        if((menu_page+1) < MAX_PAGES) {
            menu_build_tile_forward(0);
        } else {
            menu_build_corner_forward();
        }
    }

    // menu labels
    if(scrn_tile_y == 28) {
        if(scrn_tile_x >= ML_EXPL && scrn_tile_x < (ML_EXPL+4)) {
            printtext("Help", 28, ML_EXPL, MENU_HIGHLIGHT);
            idx = GAME_DOCVIEW_EXP;
        } else if(scrn_tile_x >= ML_OPTS && scrn_tile_x < (ML_OPTS+7)) {
            printtext("Options", 28, ML_OPTS, MENU_HIGHLIGHT);
            idx = GAME_OPTIONS;
        } else if(scrn_tile_x >= ML_ABOUT && scrn_tile_x < (ML_ABOUT+5)) {
            printtext("About", 28, ML_ABOUT, MENU_HIGHLIGHT);
            idx = GAME_DOCVIEW_ABOUT;
        } else {
            printtext("Help  Options  About", 28, ML_EXPL, 0x14);
            return 0;
        }

        if(mouse_buttons & 1) {
            while(mouse_buttons != 0x00) {
                asm("ldx #2");
                asm("jsr $FF6B");
                asm("sta %v", mouse_buttons);
            }

            gamestate = idx;
            return 1;
        }

    } else {
        printtext("Help  Options  About", 28, ML_EXPL, 0x14);
    }

    return 0;
}

/**
 * @brief print page number
 * 
 */
void menu_print_page() {
    char buf[16];
    sprintf(buf, "Pg:%X/%X", menu_page+1, MAX_PAGES);
    printtext(buf, 28, ML_PAGE, 0x18);
}