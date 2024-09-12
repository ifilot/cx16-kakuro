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
#include <time.h>
#include <stdio.h>
#include <conio.h>

#include "video.h"
#include "tile.h"
#include "sound.h"

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
extern uint16_t tiles_incorrect;
extern uint8_t current_puzzle_id;
extern uint8_t gamestate;
extern clock_t game_start_time;
extern char game_timebuffer[10];

/**
 * @brief Build puzzle
 * 
 */
void build_puzzle(uint8_t puzzle_id);

/**
 * @brief Show the finalized solution
 * 
 */
void show_solution();

/**
 * @brief Place a puzzle tile at location
 * 
 * @param y         y-position in puzzle
 * @param x         x-position in puzzle
 * @param tile      tile_id
 */
void set_puzzle_tile(uint8_t y, uint8_t x, uint8_t tile);

/**
 * @brief Place the solution value at location
 * 
 * @param y             y-position in puzzle
 * @param x             x-position in puzzle
 * @param tile_value    tile_value
 * @param col_id        color value
 */
void set_solution_tile(uint8_t y, uint8_t x, uint8_t tile_value, uint8_t col_id);

/**
 * @brief Handle mouse operation
 * 
 */
void puzzle_handle_mouse();

/**
 * @brief Puzzle handle keyboard interaction
 * 
 */
void puzzle_handle_keyboard();

/**
 * @brief Auxiliary function to generate puzzle clues
 * 
 */
void puzzle_generate_clues();

/**
 * @brief Auxiliary function to set revealed cells
 * 
 */
void puzzle_set_revealed_cells();

/**
 * @brief Color numbers based on correctness
 * 
 */
void puzzle_color_numbers();

/**
 * @brief Show game time
 * 
 */
void show_game_time();

/**
 * @brief Retrieve puzzle status based on puzzle id
 * 
 * @param puzzle_id 
 * @return uint8_t 
 */
uint8_t retrieve_puzzle_status(uint8_t puzzle_id);

/**
 * @brief Set the puzzle status in memory
 */
void set_puzzle_status(uint8_t puzzle_id, uint8_t status, uint8_t hours,
                       uint8_t minutes, uint8_t seconds);

/**
 * @brief Get the pointer to the puzzle status data given puzzle_Id
 * 
 * @param puzzle_id 
 * @return uint8_t* 
 */
uint8_t* get_puzzle_status_pointer(uint8_t puzzle_id);

/**
 * @brief Calculate the number of incorrect tiles
 * 
 * @return uint8_t* 
 */
uint8_t get_nr_incorrect_tiles();

/**
 * @brief Routine to invoke when the puzzle has been completed by the user
 * 
 */
void puzzle_complete();

/**
 * @brief Routine to invoke when user wants to quit the puzzle
 * 
 */
void puzzle_quit();

/**
 * @brief Auxiliary function that computes the game time
 * 
 * @param buf 
 */
void calculate_game_time();

/**
 * @brief Store the puzzle state at the end of the game
 * 
 */
void store_puzzle_state();

/**
 * @brief Wait for key to be pressed
 * 
 * @param key 
 */
void wait_for_key(uint8_t key);

/**
 * @brief Build window
 * 
 * @param y y-position
 * @param x x-position
 * @param h height
 * @param w width
 */
void build_window(uint8_t y, uint8_t x, uint8_t h, uint8_t w);

/**
 * @brief Print the border around the clock
 * 
 * @param y y-position
 * @param x x-position
 */
void print_clock_border(uint8_t y, uint8_t x);

/**
 * @brief Print clock
 * 
 * @param y y-position on screen
 * @param x x-position on screen
 */
void print_clock(const char* s, uint8_t y, uint8_t x);

/**
 * @brief Load puzzles data into memory
 * 
 */
void load_puzzles();

/**
 * @brief Save puzzles data to SD-card
 * 
 */
void save_puzzles();

#endif // _PUZZLE_H