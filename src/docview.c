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

#include "docview.h"

/**
 * @brief Initialize screen; set number of tiles and dimensions
 * 
 */
void docview_init_screen() {
    // set scaling (resolution: 640 x 480)
    VERA.display.hscale = 128;
    VERA.display.vscale = 128;

    // layer 0: background
    set_tilebase_layer0(TILEBASE_MENU);
    
    // layer 1: text
    VERA.layer1.config   = (1 << 3) | (2 << 4) | (1 << 6);      // T256C
    VERA.layer1.tilebase = (TILEBASE_FONT8 >> 9);       // 16 x 16 tiles
    VERA.layer1.mapbase  = MAPBASE1 >> 9;

    // enable both layers
    VERA.display.video |= 0b00110000;

    docview_clear_screen();
}

/**
 * @brief Clear layer0 and layer1 with background and empty tiles, respectively
 * 
 */
void docview_clear_screen() {
    fill_layer(TILE_BACKGROUND, LAYER0, PALETTEBYTE, 64, 64);
    fill_layer(0x20, LAYER1, 0x00, 64, 128);
}

/**
 * @brief Write string to screen
 * 
 * @param s pointer to null-terminated string
 */
void docview_write_string(const char* s) {
    uint8_t x = 0;
    uint8_t y = 0;
    uint32_t map_base_addr = MAPBASE1 + (y << 7 + x) * 2;

    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;
    

    while(*s != '\0') {
        VERA.data0 = *s - 0x20;
        VERA.data0 = 0x02;
        
        x++;
        if(x >= 80) {
            x = 0;
            y++;
            map_base_addr = MAPBASE1 + (y << 7 + x) * 2;
            VERA.address = map_base_addr;
            VERA.address_hi = map_base_addr >> 16;
            VERA.address_hi |= 0b10000;
        }

        s++;
    }
}

/**
 * @brief Load file into memory
 * 
 * @param filename filename on SD card
 */
void docview_load_file(const char* filename) {
    uint8_t *ptr;

    // swap to ram bank for docview
    asm("lda #%b", RAMBANK_DOCVIEW);
    asm("sta 0");

    // load file into memory
    cbm_k_setnam(filename);
    cbm_k_setlfs(0, 8, 2);
    ptr = (uint8_t*)(cbm_k_load(0, BANKED_RAM));
    *ptr = 0x00;    // write terminating byte

    // swap back to first rambank
    asm("lda 0");
    asm("sta 0");
}

/**
 * @brief Print file contents onto screen
 */
void docview_show_file() {
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t *s = (uint8_t*)(BANKED_RAM);
    uint32_t map_base_addr = MAPBASE1 + (y << 7 + x) * 2;

    // set docview ram bank
    asm("lda #%b", RAMBANK_DOCVIEW);
    asm("sta 0");

    VERA.address = map_base_addr;
    VERA.address_hi = map_base_addr >> 16;
    VERA.address_hi |= 0b10000;
    
    while(*s != '\0') {
        if(*s == 0x0A) {
            x = 0;
            y++;
            map_base_addr = MAPBASE1 + (y << 7 + x) * 2;
            VERA.address = map_base_addr;
            VERA.address_hi = map_base_addr >> 16;
            VERA.address_hi |= 0b10000;
        } else {
            VERA.data0 = *s - 0x20;
            VERA.data0 = 0x12;
        }

        s++;
    }

    // return to original ram bank
    asm("lda 0");
    asm("sta 0");
}

/**
 * @brief Handle key events
 * 
 */
void docview_handle_key() {
    static uint8_t keycode;

    // grab keycode
    asm("jsr $FFE4");
    asm("sta %v", keycode);

    if(keycode == KEYCODE_ESCAPE) {
        gamestate = 0;
    }
}