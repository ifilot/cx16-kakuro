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

#ifndef _MENU_H
#define _MENU_H

#include <stdint.h>
#include <stdio.h>

#include "video.h"
#include "constants.h"
#include "puzzle.h"

extern uint8_t menu_page;

/**
 * @brief Build the puzzle selection  menu
 * 
 */
void menu_init();

/**
 * @brief Build a menu page turning tile for the forward direction
 * 
 */
void menu_build_tile_forward(uint8_t col);

/**
 * @brief Build a menu page turning tile for the backward direction
 * 
 */
void menu_build_tile_backward(uint8_t col);

/**
 * @brief Build regular right-bottom page corner
 * 
 */
void menu_build_corner_forward();

/**
 * @brief Build regular left-bottom page corner
 * 
 */
void menu_build_corner_backward();

/**
 * @brief Build a menu page turning tile for the backward direction
 * 
 */
void menu_build_icon(uint8_t y, uint8_t x, uint8_t puzzle_id, uint8_t select);

/**
 * @brief Handle mouse operation
 * 
 */
uint8_t menu_handle_mouse();

/**
 * @brief print page number
 * 
 */
void menu_print_page();

#endif // _MENU_H