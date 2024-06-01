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

/**
 * @brief Build puzzle
 * 
 */
void build_puzzle() {
    uint8_t rows, cols, i, j, idx, ctr, c;
    uint8_t *v = (uint8_t*)0xA000;

    // load puzzle into memory
    cbm_k_setnam("puzzle.dat");
    cbm_k_setlfs(0, 8, 2);
    cbm_k_load(0, 0xA000);

    rows = (*v >> 4) & 0x0F;
    cols = *v & 0x0F;

    ctr = 0;
    for(i=0; i<rows; i++) {
        for(j=0; j<cols; j++) {
            switch(ctr) {
                case 0:
                    c = (*v >> 4) & 0x0F;
                break;
                case 1:
                    c = *v & 0x0F;
                break;
            }

            if(c == 0) {
                set_tile(i*2, j*2, TILE_BLOCKED1, 0x00);
                set_tile(i*2+1, j*2, TILE_BLOCKED2, (1 << 2) | (1 << 3));
                set_tile(i*2, j*2+1, TILE_BLOCKED2, 0x00);
                set_tile(i*2+1, j*2+1, TILE_BLOCKED1, (1 << 2) | (1 << 3));
            } else {
                set_tile(i*2, j*2, TILE_EMPTY, 0x00);
                set_tile(i*2+1, j*2, TILE_EMPTY, (1 << 3));
                set_tile(i*2, j*2+1, TILE_EMPTY, (1 << 2));
                set_tile(i*2+1, j*2+1, TILE_EMPTY, (1 << 2) | (1 << 3));
            }

            ctr++;
            if(ctr == 2) {
                ctr = 0;
                v++;
            }
        }
    }
}