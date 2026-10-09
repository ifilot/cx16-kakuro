#include "resource.h"
#include <cbm.h>
#include <cx16.h>
#include <stdint.h>

#include "constants.h"
#include "splash.h"
#include "sound.h"
#include "menu.h"
#include "puzzle.h"
#include "transition.h"

void show_start_screen(void) {
    uint8_t video = VERA.display.video;
    uint8_t palette[8];
    uint8_t i;

    /* Hide both layers and sprites while loading over the boot screen. */
    VERA.display.video = video & 0x0F;
    if(!resource_vram(RES_SPLASH0,0)) {
        VERA.display.video = video;
        return;
    }
    if(!resource_vram(RES_SPLASH1,0xF000)) {
        VERA.display.video = video;
        return;
    }

    /* Temporarily replace palette entries 0..3, preserving the game colors. */
    VERA.control = 0;
    VERA.address = 0xFA00;
    VERA.address_hi = 0x11;
    for(i = 0; i < sizeof(palette); i++) {
        palette[i] = VERA.data0;
    }
    if(!resource_vram(RES_SPLASHP,0x1FA00UL)) {
        VERA.address = 0xFA00;
        VERA.address_hi = 0x11;
        for(i = 0; i < sizeof(palette); i++) {
            VERA.data0 = palette[i];
        }
        VERA.display.video = video;
        return;
    }

    /* 2bpp bitmap at VRAM $00000, 640 pixels wide, at native resolution. */
    VERA.layer0.config = 0x05;
    VERA.layer0.tilebase = 1;
    VERA.layer0.hscroll = 0;
    VERA.layer0.vscroll = 0;
    VERA.display.hscale = 128;
    VERA.display.vscale = 128;
    VERA.display.video = (video & 0x0F) | 0x10;

    sound_scene(MUSIC_MENU);

    /* Ignore any keys buffered during boot. */
    while(cbm_k_getin() != 0) {}
    load_puzzles();
    menu_prepare_cache();
    load_tiles(RES_JCURSOR,0x1E000UL);
    /* Banks 3 and lower 4 hold the persistent quit bitmap; effects use upper 4. */
    i=*(volatile uint8_t*)0;
    *(volatile uint8_t*)0=3;
    resource_load(RES_GQUIT,BANKED_RAM,0);
    *(volatile uint8_t*)0=i;
    while(cbm_k_getin() != KEYCODE_RETURN) {sound_fill_buffers();}
    play_sfx(SFX_SELECT);

    transition_begin();
}
