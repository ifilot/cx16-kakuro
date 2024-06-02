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

#include "tile.h"

static const uint8_t rambank_tile = RAMBANK_TILE;

/**
 * @brief Load small digits into memory
 * 
 */
void load_small_digits() {
    uint32_t map_base_addr;
    uint8_t *ptr = (uint8_t*)(TILE_SRC);
    uint16_t i = 0;

    // set ram bank
    asm("lda %v", rambank_tile);
    asm("sta 0");

    // load small digits into memory
    cbm_k_setnam("sdigits.dat");
    cbm_k_setlfs(0, 8, 2);
    cbm_k_load(0, SMALL_DIGIT_BUFFER);

    map_base_addr = TILEBASE + 256 * TILE_CLUE2;
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;

    for(i=0; i<256; i++) {
        (*ptr++) = VERA.data0;
    }

    // swap back to default ram bank
    asm("lda 0");
    asm("sta 0");
}

/**
 * @brief Build a new tile and upload it to VRAM
 * 
 */
void build_clue_tile_right(uint16_t vrampos, uint8_t value) {
    uint32_t map_base_addr;
    uint8_t *ptr_src = (uint8_t*)(TILE_BUFFER-1);
    uint8_t *ptr_des = (uint8_t*)(TILE_BUFFER);
    uint16_t i;
    uint8_t k,l,x,q;
    char buf[3];

    // set ram bank
    asm("lda %v", rambank_tile);
    asm("sta 0");

    // copy with reverse ordering (double-mirroring)
    for(i=0; i<256; i++) {
        *ptr_des++ = *ptr_src--;
    }

    // convert number to string
    itoa(value, buf, 10);

    // copy digits onto tile
    for(x=0; x<3; x++) {
        if(buf[x] == '\0') {
            break;
        }

        ptr_src = (uint8_t*)(SMALL_DIGIT_BUFFER + (buf[x] - '0') * 32);
        for(k=0; k<8; k++) {

            if(buf[1] == '\0') {
                ptr_des = (uint8_t*)(TILE_BUFFER + (k+4) * 16 + 8);
            } else {
                ptr_des = (uint8_t*)(TILE_BUFFER + (k+4) * 16 + 3 + x * 5);
            }
            
            for(l=0; l<4; l++) {
                q = *ptr_src++;
                if(q != 0x00) {
                    *ptr_des = q;
                }
                ptr_des++;
            }
        }
    }


    // reset pointer and copy to VRAM
    ptr_des = (uint8_t*)(0xA000 + 8 * 4 * 10 + 256);
    map_base_addr = TILEBASE + 256 * vrampos;
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;

    // copy to VRAM
    for(i=0; i<256; i++) {
        VERA.data0 = *ptr_des++;
    }

    // swap back to default ram bank
    asm("lda 0");
    asm("sta 0");
}

/**
 * @brief Build a new tile and upload it to VRAM
 * 
 */
void build_clue_tile_down(uint16_t vrampos, uint8_t value) {
    uint32_t map_base_addr;
    uint8_t *ptr_src = (uint8_t*)(TILE_BUFFER-1);
    uint8_t *ptr_des = (uint8_t*)(TILE_BUFFER);
    uint16_t i;
    uint8_t k,l,x,q;
    char buf[3];

    // set ram bank
    asm("lda %v", rambank_tile);
    asm("sta 0");

    // create a copy of the down-clue tile
    memcpy((void*)TILE_BUFFER, (void*)TILE_SRC, 256);

    // convert number to string
    itoa(value, buf, 10);

    // copy digits onto tile
    ptr_des = (uint8_t*)(TILE_BUFFER);
    for(x=0; x<3; x++) {
        if(buf[x] == '\0') {
            break;
        }

        ptr_src = (uint8_t*)(SMALL_DIGIT_BUFFER + (buf[x] - '0') * 32);
        for(k=0; k<8; k++) {
            ptr_des = (uint8_t*)(TILE_BUFFER + (k+4) * 16 + (5 + x * 5));
            for(l=0; l<4; l++) {
                q = *ptr_src++;
                if(q != 0x00) {
                    *ptr_des = q;
                }
                ptr_des++;
            }
        }
    }

    // reset pointer and copy to VRAM
    ptr_des = (uint8_t*)(0xA000 + 8 * 4 * 10 + 256);
    map_base_addr = TILEBASE + 256 * vrampos;
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;

    // copy to VRAM
    for(i=0; i<256; i++) {
        VERA.data0 = *ptr_des++;
    }

    // swap back to default ram bank
    asm("lda 0");
    asm("sta 0");
}