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

#include "video.h"

/**
 * MEMORY MAP
 * ==========
 * 0x0000 - 0x6000 - Tileset for menu
 * 0x6000 - 0x8000 - Tileset for game
 * 
 */

/**
 * @brief Initialize screen
 * 
 */
void init_screen() {
    // set scaling (resolution: 640 x 480)
    VERA.display.hscale = 128;
    VERA.display.vscale = 128;

    // layer 0
    // This layer contains the game background and the puzzle background; this
    // includes all 'static' squares
    set_tilebase_layer0(TILEBASE_MENU);
    
    // layer 1
    // On this layer, the values that the user fills in for the tiles are shown
    VERA.layer1.config   = 0x03 | (1 << 4) | (1 << 6);          // 8 bpp color depth
    VERA.layer1.tilebase = (TILEBASE_MENU >> 9) & 0xF8 | 0x03;  // 16 x 16 tiles
    VERA.layer1.mapbase  = MAPBASE1 >> 9;

    // enable both layers
    VERA.display.video |= 0b00110000;

    // enable sprites
    //VERA.display.video |= 0b01000000;
}

/**
 * @brief Set the tilebase offset for layer0
 * 
 * @param addr VRAM address
 */
void set_tilebase_layer0(uint16_t addr) {
    VERA.layer0.config   = 0x03 | (1 << 4) | (1 << 6);      // 8 bpp color depth
    VERA.layer0.tilebase = (addr >> 9) & 0xF8 | 0x03;       // 16 x 16 tiles
    VERA.layer0.mapbase  = MAPBASE0 >> 9;                   // upper 8 bits
}

/**
 * @brief Load the tiles from file into video memory
 * 
 */
void load_tiles(const char* filename, uint16_t addr) {
    cbm_k_setnam(filename);
    cbm_k_setlfs(0, 8, 2);
    cbm_k_load(2, addr);
}

/**
 * @brief Clears the screen
 * 
 * Set background tiles everywhere and sets foreground to transparent
 * 
 */
void clear_screen() {
    fill_layer(TILE_BACKGROUND, LAYER0);
    fill_layer(TILE_NONE, LAYER1);
}

/**
 * @brief Fill layer 0 with all tiles of the same type
 *
 * @param tile_id background tile index
 * @param layer   which layer to fill
 */
void fill_layer(uint8_t tile_id, uint8_t layer) {
    uint8_t i,j,k;
    uint32_t map_base_addr;

    // set background tiles
    map_base_addr = (layer == 0 ? MAPBASE0 : MAPBASE1);
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;

    for (j=0; j<MAPHEIGHT; j++) {
        for (i=0; i<MAPWIDTH; i++) {
            VERA.data0 = tile_id;       // background tile
            k = PALETTEBYTE;
            if(j % 2 == 1) {
                k |= (1 << 3);
            }

            if(i % 2 == 1) {
                k |= (1 << 2);
            }

            VERA.data0 = k;   // palette offset data
        }
    }
}

/**
 * @brief Set a tile on LAYER1
 * 
 * @param y             y-position of the tile
 * @param x             x-position of the tile
 * @param tile_id       which tile to place
 * @param tile_data     tile placement byte
 * @param layer         which layer to place tile on
 */
void set_tile(uint8_t y, uint8_t x, uint8_t tile_id, uint8_t tile_data, uint8_t layer) {
    uint32_t map_base_addr = (layer == 0 ? MAPBASE0 : MAPBASE1);
    map_base_addr += (y * MAPWIDTH + x) * 2;
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;
    VERA.data0 = tile_id;
    VERA.data0 = tile_data;
}

/**
 * @brief Swap colors inside VRAM for fonts
 * 
 * @param col 
 */
void swap_color_font_tiles(uint8_t col1, uint8_t col2) {
    static const uint8_t rambank_colswap = RAMBANK_COLSWAP;
    uint8_t i;
    uint8_t *ptr;
    uint8_t *end;
    uint32_t vera_addr = 0;

    // set ram bank
    asm("lda %v", rambank_colswap);
    asm("sta 0");

    for(i=0; i<4; i++) {
        if(i < 2) {
            end = (uint8_t*)(BANKED_RAM + 16*16*16);
        } else {
            end = (uint8_t*)(BANKED_RAM + 16*16*2);
        }

        vera_addr = TILEBASE_FONT + (i+2) * 16 * 16 * 16;
        VERA.address = vera_addr;
        VERA.address_hi = vera_addr >> 16;
        VERA.address_hi |= 0b10000;
        ptr = (uint8_t*)(BANKED_RAM);

        while(ptr != (uint8_t*)(BANKED_RAM + 16*16*16)) {
            *ptr = VERA.data0;
            if(*ptr == col1) {
                *ptr = col2;
            }
            ptr++;
        }

        vera_addr = TILEBASE_FONT + (i+2) * 16 * 16 * 16;
        VERA.address = vera_addr;
        VERA.address_hi = vera_addr >> 16;
        VERA.address_hi |= 0b10000;
        ptr = (uint8_t*)(BANKED_RAM);

        while(ptr != (uint8_t*)(BANKED_RAM + 16*16*16)) {
            VERA.data0 = *ptr++;
        }
    }

    // swap back to default ram bank
    asm("lda 0");
    asm("sta 0");
}

void write_debug(const char* s) {
    uint8_t v = *s;
    uint8_t x = 0;
    while(*s != '\0') {
        v = *s;
        if(v >= '0' && v <= '9') {
            v -= '0';
        } else {
            v = v - 'A' + 10;
        }

        if(v < 8) {
            v = v * 2 + 0x20;
        } else {
            v = (v - 8) * 2 + 0x40;
        }
        set_tile(0, x*2, v, 0x00, 1);
        set_tile(0, x*2+1, v + 1, 0x00, 1);
        set_tile(1, x*2, v + 0x10, 0x00, 1);
        set_tile(1, x*2+1, v + 0x11, 0x00, 1);

        s++;
        x++;
    }
}