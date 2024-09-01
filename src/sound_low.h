/**************************************************************************
 *                                                                        *
 *   Author: Ivo Filot <ivo@ivofilot.nl>                                  *
 *                                                                        *
 *   CX16-OTHELLO is free software:                                       *
 *   you can redistribute it and/or modify it under the terms of the      *
 *   GNU General Public License as published by the Free Software         *
 *   Foundation, either version 3 of the License, or (at your option)     *
 *   any later version.                                                   *
 *                                                                        *
 *   CX16-OTHELLO is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty          *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.              *
 *   See the GNU General Public License for more details.                 *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program.  If not, see http://www.gnu.org/licenses/.  *
 *                                                                        *
 **************************************************************************/

#ifndef _SOUND_LOW_H
#define _SOUND_LOW_H

/**
 * @brief Initialize sound engine
 * 
 */
void __fastcall__ init_sound();

/**
 * @brief Start background music
 * 
 */
void __fastcall__ start_bgmusic();

/**
 * @brief Stop background music
 * 
 */
void __fastcall__ stop_bgmusic();

/**
 * @brief Rewind background music
 * 
 */
void __fastcall__ rewind_bgmusic();

/**
 * @brief Fill sound buffers
 * 
 */
void __fastcall__ sound_fill_buffers_asm();

/**
 * @brief Play short sound when placing down a stone
 * 
 */
void __fastcall__ play_thumb();

#endif // _SOUND_LOW_H