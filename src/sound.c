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

#include "sound.h"

#include <cbm.h>
uint8_t music = YES;
uint8_t sound_current_scene = 255;
static const uint16_t digit_sounds[9] = {SFX_PLACE1,SFX_PLACE2,SFX_PLACE3,
    SFX_PLACE4,SFX_PLACE5,SFX_PLACE6,SFX_PLACE7,SFX_PLACE8,SFX_PLACE9};

void sound_fill_buffers(void) {
    /* Effects and the music stream remain serviced with Music switched off. */
    sound_fill_buffers_asm();
}
void sound_load_effects(void) {
    uint8_t bank=*(volatile uint8_t*)0;
    *(volatile uint8_t*)0=RAMBANK_SFX;
    cbm_k_setnam(SFX_BANK_FILE);cbm_k_setlfs(0,8,2);
    cbm_k_load(0,SOUND_EFFECT_ADDRESS);
    *(volatile uint8_t*)0=bank;
}
void sound_scene(uint8_t scene) {
    if(scene==sound_current_scene)return;
    sound_current_scene=scene;
    /* Open the selected track even when muted, so re-enabling uses this scene. */
    play_music(scene==MUSIC_GAME ? "game.zsm" : "menu.zsm");
    if(!music)stop_bgmusic();
    sound_fill_buffers();
}
void sound_digit(uint8_t digit) {
    if(digit>=1 && digit<=9)play_sfx(digit_sounds[digit-1]);
}
