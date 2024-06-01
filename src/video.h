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
void load_tiles();

/**
 * @brief Clears the screen
 * 
 * Set background tiles everywhere and sets foreground to transparent
 * 
 */
void clear_screen();

/**
 * @brief Which tile to use for the background
 * 
 * @param tile_id background tile index
 */
void set_background(uint8_t tile_id);

/**
 * @brief Set all foreground tiles (LAYER1) to transparent
 * 
 */
void clear_foreground();

/**
 * @brief Set a tile on LAYER1
 * 
 * @param y             y-position of the tile
 * @param x             x-position of the tile
 * @param tile_id       which tile to place
 * @param tile_data     tile placement byte
 */
void set_tile(uint8_t y, uint8_t x, uint8_t tile_id, uint8_t tile_data);

#endif // _VIDEO_H