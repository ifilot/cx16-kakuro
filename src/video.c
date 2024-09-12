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

    // layer 0
    // This layer contains the game background and the puzzle background; this
    // includes all 'static' squares
    set_tilebase_layer0(TILEBASE_MENU);
    
    // layer 1
    // On this layer, the values that the user fills in for the tiles are shown
    VERA.layer1.config   = (1 << 3) | (1 << 4) | (1 << 6);      // T256C
    VERA.layer1.tilebase = (TILEBASE_FONT16 >> 9) | 0x03;       // 16 x 16 tiles
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
    VERA.layer0.tilebase = (addr >> 9) | 0x03;              // 16 x 16 tiles
    VERA.layer0.mapbase  = MAPBASE0 >> 9;                   // upper 8 bits
}

/**
 * @brief Load the tiles from file into video memory
 * 
 */
void load_tiles(const char* filename, uint32_t addr) {
    cbm_k_setnam(filename);
    cbm_k_setlfs(0, 8, 2);
    cbm_k_load(2 + (addr >> 16), addr);
}

/**
 * @brief Clears the screen
 * 
 * Set background tiles everywhere and sets foreground to transparent
 * 
 */
void clear_screen() {
    fill_layer(TILE_BACKGROUND, LAYER0, PALETTEBYTE, 64, 64);
    fill_layer(0x20, LAYER1, 0x00, 64, 64);
}

/**
 * @brief Fill layer 0 with all tiles of the same type
 *
 * @param tile_id background tile index
 * @param layer   which layer to fill
 * @param b2      which tile info to use
 * @param height  height of the map
 * @param width   width of the map
 */
void fill_layer(uint8_t tile_id, uint8_t layer, uint8_t b2, uint8_t height, uint8_t width) {
    uint8_t i,j;
    uint32_t map_base_addr;

    // set background tiles
    map_base_addr = (layer == 0 ? MAPBASE0 : MAPBASE1);
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;

    for (j=0; j<height; j++) {
        for (i=0; i<width; i++) {
            VERA.data0 = tile_id;       // background tile
            VERA.data0 = b2;            // palette offset data
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

        vera_addr = TILEBASE_FONT16 + (i+2) * 16 * 16 * 16;
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

        vera_addr = TILEBASE_FONT16 + (i+2) * 16 * 16 * 16;
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
    uint8_t x = 0;
    while(*s != '\0') {
        set_tile(0, x+1, *s, 0x10, LAYER1);
        s++;
        x++;
    }
}

/**
 * @brief Print text to the screen
 * 
 * @param s string
 * @param y y-position
 * @param x x-position
 * @param col color id
 */
void printtext(const char* s, uint8_t y, uint8_t x, uint8_t col) {
    while(*s != 0) {
        set_tile(y, x, *s, col, LAYER1);
        s++;
        x++;
    }
}

/**
 * @brief Print spaces to the screen
 * 
 * @param nrspaces number of spaces
 * @param y y-position
 * @param x x-position
 */
void printspaces(uint8_t nrspaces, uint8_t y, uint8_t x) {
    uint8_t i = 0;
    for(i=0; i<nrspaces; i++) {
        set_tile(y, x+i, 0x20, 0x00, LAYER1);
    }
}

/**
 * @brief Save the current screen state
 * 
 */
void save_screen_state() {
    uint8_t *ptr = (uint8_t*)(0xA000);
    uint32_t map_base_addr;
    uint16_t i;

    // fill sound buffer before performing this relatively expensive function
    sound_fill_buffers();

    asm("lda #%b", RAMBANK_SCREEN);
    asm("sta 0");

    map_base_addr = MAPBASE0;
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;
    for(i=0; i<0x1000; i++) {
        *(ptr++) = VERA.data0;
    }

    map_base_addr = MAPBASE1;
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;
    for(i=0; i<0x1000; i++) {
        *(ptr++) = VERA.data0;
    }

    asm("lda 0");
    asm("sta 0");
}

/**
 * @brief Restore the current screen state
 * 
 */
void restore_screen_state() {
    uint8_t *ptr = (uint8_t*)(0xA000);
    uint32_t map_base_addr;
    uint16_t i;

    asm("lda #%b", RAMBANK_SCREEN);
    asm("sta 0");

    map_base_addr = MAPBASE0;
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;
    for(i=0; i<0x1000; i++) {
        VERA.data0 = (*ptr++);
    }

    map_base_addr = MAPBASE1;
    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;
    for(i=0; i<0x1000; i++) {
        VERA.data0 = (*ptr++);
    }

    asm("lda 0");
    asm("sta 0");
}

/**
 * @brief Set the mouse pointer
 * 
 * @param tile_id   which tile to use
 */
void set_mouse_pointer(uint8_t tile_id) {
    unsigned long sprite_addr = 0x1FC00;
    uint32_t graph_addr = TILEBASE_MENU + (tile_id << 8);

    VERA.address = sprite_addr;
    VERA.address_hi = sprite_addr >> 16;
    VERA.address_hi |= 0b10000;

    VERA.data0 = graph_addr >> 5;
    VERA.data0 = (graph_addr >> 13) | (1 << 7);
}