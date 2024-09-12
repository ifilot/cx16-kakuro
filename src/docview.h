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

#ifndef _DOCVIEW_H
#define _DOCVIEW_H

#include <ascii_charmap.h>

#include "puzzle.h"
#include "video.h"
#include "sound.h"

/**
 * @brief Initialize screen
 * 
 */
void docview_init_screen();

/**
 * @brief Prepare screen to write text to it
 * 
 */
void docview_clear_screen();

/**
 * @brief Write string to screen
 * 
 * @param s 
 */
void docview_write_string(const char* s);

/**
 * @brief Load file into memory
 * 
 * @param s 
 */
void docview_load_file(const char* s);

/**
 * @brief Print file contents onto screen
 */
void docview_show_file();

/**
 * @brief Handle key events
 * 
 */
void docview_handle_key();

#endif // _DOCVIEW_H