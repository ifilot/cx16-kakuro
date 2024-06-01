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
 * @brief Initialize screen
 * 
 */
void init_screen() {
    // set scaling (resolution: 640 x 480)
    VERA.display.hscale = 128;
    VERA.display.vscale = 128;

    // tile configuration
    VERA.layer0.config   = 0x03 | (1 << 4) | (1 << 6);  // 8 bpp color depth
    VERA.layer0.tilebase = (TILEBASE >> 11) | 0x03;     // 16 x 16 tiles
    VERA.layer0.mapbase  = MAPBASE0 >> 9;               // upper 8 bits
    
    VERA.layer1.config   = 0x03 | (1 << 4) | (1 << 6);  // 8 bpp color depth
    VERA.layer1.tilebase = (TILEBASE >> 11) | 0x03;     // 16 x 16 tiles
    VERA.layer1.mapbase  = MAPBASE1 >> 9;

    // enable both layers
    VERA.display.video |= 0b00110000;

    // enable sprites
    //VERA.display.video |= 0b01000000;
}

/**
 * @brief Load the tiles from file into memory
 * 
 */
void load_tiles() {
    // location of tile set
    uint32_t addr = 0x0000;

    // counters
    uint8_t i = 0, j=0;

    // load tiles into memory
    cbm_k_setnam("tiles.dat");
    cbm_k_setlfs(0, 8, 2);
    cbm_k_load(2, TILEBASE);
}

/**
 * @brief Clears the screen
 * 
 * Set background tiles everywhere and sets foreground to transparent
 * 
 */
void clear_screen() {
    // set background
    set_background(TILE_BACKGROUND);

    // set all foreground tiles to transparent
    clear_foreground();
}

/**
 * @brief Which tile to use for the background
 * 
 * @param tile_id background tile index
 */
void set_background(uint8_t tile_id) {
    uint8_t i = 0, j=0;
    uint32_t map_base_addr;

    // set background tiles
    map_base_addr = MAPBASE0;
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;

    for (j=0; j<MAPHEIGHT; j++) {
        for (i=0; i<MAPWIDTH; i++) {
            VERA.data0 = tile_id;       // background tile
            VERA.data0 = PALETTEBYTE;   // palette offset data
        }
    }
}

/**
 * @brief Set all foreground tiles (LAYER1) to transparent
 * 
 */
void clear_foreground() {
    uint8_t i = 0, j=0;
    uint32_t map_base_addr;

    // set all foreground positions to transparent
    map_base_addr = MAPBASE1;
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;

    for (j=0; j<MAPHEIGHT; j++) {
        for (i=0; i<MAPWIDTH; i++) {
            VERA.data0 = TILE_NONE;        // transparent tile
            VERA.data0 = PALETTEBYTE;      // palette offset data
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
 */
void set_tile(uint8_t y, uint8_t x, uint8_t tile_id, uint8_t tile_data) {
    uint32_t map_base_addr = MAPBASE1 + (y * MAPWIDTH + x) * 2;
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;
    VERA.data0 = tile_id;
    VERA.data0 = tile_data;
}