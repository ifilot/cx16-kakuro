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

#ifndef _SOUND_H
#define _SOUND_H

#include <stdint.h>

#include "constants.h"
#include "sound_low.h"
#include "sfx.h"

#define MUSIC_MENU 0
#define MUSIC_GAME 1
#define RAMBANK_SFX 4
#define SOUND_EFFECT_ADDRESS 0xB000
#if SFX_BANK_SIZE > 4096
#error Sound effects exceed the reserved half-bank
#endif

extern uint8_t music;               // whether to play music

/**
 * @brief Fill sound buffer
 * 
 */
void sound_fill_buffers(void);
void sound_load_effects(void);
void sound_scene(uint8_t scene);
void sound_digit(uint8_t digit);

#endif // _SOUND_H