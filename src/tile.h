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

#ifndef _TILE_H
#define _TILE_H

#include <stdint.h>
#include <cbm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ascii_charmap.h>

#include "constants.h"

/**
 * @brief Load small digits into memory
 * 
 */
void load_small_digits();

/**
 * @brief Build a new tile and upload it to VRAM
 * 
 */
void build_clue_tile_down(uint16_t vrampos, uint8_t value);

/**
 * @brief Build a new tile and upload it to VRAM
 * 
 */
void build_clue_tile_right(uint16_t vrampos, uint8_t value);

#endif // _TILE_H