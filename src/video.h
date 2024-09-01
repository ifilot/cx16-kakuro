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

#ifndef _VIDEO_H
#define _VIDEO_H

#include <cx16.h>
#include <cbm.h>
#include <ascii_charmap.h>
#include <stdint.h>

#include "constants.h"

/**
 * @brief Initialize screen
 * 
 */
void init_screen();

/**
 * @brief Load the tiles from file into memory
 * 
 */
void load_tiles(const char* filename, uint32_t addr);

/**
 * @brief Set the tilebase offset for layer0
 * 
 * @param addr 
 */
void set_tilebase_layer0(uint16_t addr);

/**
 * @brief Clears the screen
 * 
 * Set background tiles everywhere and sets foreground to transparent
 * 
 */
void clear_screen();

/**
 * @brief Fill layer 0 with all tiles of the same type
 *
 * @param tile_id background tile index
 * @param layer   which layer to fill
 */
void fill_layer(uint8_t tile_id, uint8_t layer, uint8_t b2);

/**
 * @brief Set a tile on LAYER1
 * 
 * @param y             y-position of the tile
 * @param x             x-position of the tile
 * @param tile_id       which tile to place
 * @param tile_data     tile placement byte
 * @param layer         which layer to place tile on
 */
void set_tile(uint8_t y, uint8_t x, uint8_t tile_id, uint8_t tile_data, uint8_t layer);

/**
 * @brief Change the color of the big font
 * 
 * @param col 
 */
void swap_color_font_tiles(uint8_t col1, uint8_t col2);

/**
 * @brief Write debug message to screen
 * 
 * @param s 
 */
void write_debug(const char* s);

/**
 * @brief Print text to the screen
 * 
 * @param s string
 * @param y y-position
 * @param x x-position
 * @param col color id
 */
void printtext(const char* s, uint8_t y, uint8_t x, uint8_t col);

/**
 * @brief Print spaces to the screen
 * 
 * @param nrspaces number of spaces
 * @param y y-position
 * @param x x-position
 */
void printspaces(uint8_t nrspaces, uint8_t y, uint8_t x);

/**
 * @brief Save the current screen state
 * 
 */
void save_screen_state();

/**
 * @brief Restore the current screen state
 * 
 */
void restore_screen_state();

/**
 * @brief Set the mouse pointer
 * 
 * @param tile_id   which tile to use
 */
void set_mouse_pointer(uint8_t tile_id);

#endif // _VIDEO_H