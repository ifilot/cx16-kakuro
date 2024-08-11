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

void build_menu() {
    uint8_t i,j,k,l;
    static const uint8_t offset_x = 4;
    static const uint8_t offset_y = 4;

    k = 1;
    l = 0;
    for(i=0; i<8; i++) {
        for(j=0; j<8; j++) {
            set_tile(offset_y + i*3, offset_x + j*4, TILE_BTN_CORNER, 0x00, 0);
            set_tile(offset_y + i*3, offset_x + j*4+1, TILE_BTN_EDGE_UD, 0x00, 0);
            set_tile(offset_y + i*3, offset_x + j*4+2, TILE_BTN_EDGE_UD, 0x00, 0);
            set_tile(offset_y + i*3, offset_x + j*4+3, TILE_BTN_CORNER, MIRROR_X, 0);
            
            set_tile(offset_y + i*3+1, offset_x + j*4, TILE_BTN_EDGE_LR, 0x00, 0);
            set_tile(offset_y + i*3+1, offset_x + j*4+1, TILE_BTN_CTR, 0x00, 0);
            set_tile(offset_y + i*3+1, offset_x + j*4+2, TILE_BTN_CTR, 0x00, 0);
            set_tile(offset_y + i*3+1, offset_x + j*4+3, TILE_BTN_EDGE_LR, MIRROR_X, 0);

            set_tile(offset_y + i*3+2, offset_x + j*4, TILE_BTN_CORNER, MIRROR_Y, 0);
            set_tile(offset_y + i*3+2, offset_x + j*4+1, TILE_BTN_EDGE_UD, MIRROR_Y, 0);
            set_tile(offset_y + i*3+2, offset_x + j*4+2, TILE_BTN_EDGE_UD, MIRROR_Y, 0);
            set_tile(offset_y + i*3+2, offset_x + j*4+3, TILE_BTN_CORNER, MIRROR_XY, 0);

            if(k == 10) {
                k = 0;
            }

            set_tile(offset_y + i*3+1, offset_x + j*4+1, 0x10+l, 0x00, 1);
            set_tile(offset_y + i*3+1, offset_x + j*4+2, 0x10+k, 0x00, 1);

            if(k == 0) {
                l++;
            }

            k++;
        }
    }
}