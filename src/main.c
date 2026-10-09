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
#include "docview.h"
#include "puzzle.h"

#include "tile.h"
#include "mouse.h"
#include "menu.h"
#include "sound.h"
#include "splash.h"
#include "playfield.h"

void main() {
    init_sound();
    sound_load_effects();
    show_start_screen();

    // The journal RAM caches and puzzle data were prepared behind the title.
    if(!puzzle_filesize)load_puzzles();
    init_mouse();
    while(1) {
        /***********************************************************************
         * MENU
         **********************************************************************/
        menu_init();
        sound_scene(MUSIC_MENU);
        while(menu_handle_mouse() == 0) {
            sound_fill_buffers();
        }
        menu_leave();

        /***********************************************************************
         * DOCVIEWER
         **********************************************************************/
        if((gamestate & GAME_DOCVIEW_EXP) || (gamestate & GAME_DOCVIEW_ABOUT)) {
            docview_init_screen();
            if(gamestate == GAME_DOCVIEW_EXP) {
                docview_load_file("HELP.TXT");
            }
            if(gamestate == GAME_DOCVIEW_ABOUT) {
                docview_load_file("ABOUT.TXT");
            }
            docview_show_file();
            while((gamestate & GAME_DOCVIEW_EXP) || (gamestate & GAME_DOCVIEW_ABOUT)) {
                docview_handle_key();
                sound_fill_buffers();
            }
            docview_leave();
        }

        /***********************************************************************
         * GAME
         **********************************************************************/
        if(gamestate & GAME_PLAY) {
            build_puzzle(current_puzzle_id);
            sound_scene(MUSIC_GAME);
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
            playfield_leave();
        }
    }
}
