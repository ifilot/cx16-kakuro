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

#ifndef _PUZZLE_H
#define _PUZZLE_H

#include <stdint.h>
#include <cbm.h>
#include <stdlib.h>

#include "video.h"
#include "tile.h"

extern uint8_t* puzzledata;
extern uint8_t puzzlerows;
extern uint8_t puzzlecols;
extern uint8_t puzzlecells;
extern uint8_t offset_x;
extern uint8_t offset_y;
extern int8_t ccurx;
extern int8_t ccury;
extern int8_t ocurx;
extern int8_t ocury;

/**
 * @brief Build puzzle
 * 
 */
void build_puzzle();

void set_puzzle_tile(uint8_t y, uint8_t x, uint8_t tile);

void puzzle_handle_mouse();

#endif // _PUZZLE_H