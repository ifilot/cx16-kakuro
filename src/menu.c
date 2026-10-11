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

#include "menu.h"
#include "resource.h"
#include "transition.h"

#define HELP_BUTTON 24
#define OPTIONS_BUTTON 25
#define ABOUT_BUTTON 26
#define PREVIOUS_BUTTON 27
#define NEXT_BUTTON 28
#define MUSIC_BUTTON 29
#define BACK_BUTTON 30
#define PAGE_SIZE 24
#define COLUMNS 6

uint8_t menu_page = 0;
static uint8_t selected = 0;
static uint8_t active = 0;
static uint8_t cached_page = 255;
static uint8_t options_open = 0;
static uint8_t controls_dirty=0;
static uint8_t statuses[PAGE_SIZE];
static int8_t hovered = -1;
static int8_t pressed = -1;
static uint8_t previous_buttons = 0;

/* The bitmap occupies $00000..$12BFF. Fonts/maps remain above that range;
 * a private cursor at $1E000 survives overwriting the old menu tile set. */
static void bitmap_address(uint16_t x, uint16_t y, uint8_t increment) {
    uint16_t large = y << 7;
    uint16_t address = large + (y << 5);
    uint8_t high = address < large;
    uint16_t column = x >> 2;
    large = address + column;
    if(large < address) high++;
    VERA.address = large;
    VERA.address_hi = high | increment;
}

static uint16_t card_x(uint8_t column) {
    return 100 + column * 72 + (column >= 3 ? 24 : 0);
}

extern const uint8_t* menu_blit_source;
extern uint8_t menu_blit_width, menu_blit_height;
extern void menu_blit(void);

static void cached_rectangle(uint8_t bank, uint16_t offset, uint16_t x,
                             uint16_t y, uint8_t width, uint8_t height) {
    uint8_t old_bank = *(volatile uint8_t*)0;
    *(volatile uint8_t*)0 = bank;
    menu_blit_source = (const uint8_t*)(BANKED_RAM + offset);
    menu_blit_width = width;
    menu_blit_height = height;
    VERA.control = 0;
    bitmap_address(x, y, 0x10);
    menu_blit();
    *(volatile uint8_t*)0 = old_bank;
}

static void draw_card(uint8_t index) {
    uint16_t x = card_x(index % COLUMNS);
    uint16_t y = 112 + (index / COLUMNS) * 56;
    uint8_t status = statuses[index];
    uint8_t state = index == selected ? 1 : (status & STATUS_SOLVED ? 3 : (status & STATUS_OPENED ? 2 : 0));
    cached_rectangle(16 + state * 3 + index / 8, (index & 7) * 1024,
                     x - 4, y - 4, 16, 56);
    if(index == selected && (status & (STATUS_SOLVED | STATUS_OPENED)))
        cached_rectangle(10, status & STATUS_SOLVED ? 2048 : 0,
                         x + 44, y + 2, 3, 12);
}

static void draw_details(void) {
    uint8_t status = statuses[selected];
    uint8_t state = status & STATUS_SOLVED ? 2 : (status & STATUS_OPENED ? 1 : 0);
    cached_rectangle(28 + state * 12 + selected / 2, (selected & 1) * 4096,
                     96, 344, 112, 21);
}

static void cached_button(uint8_t id, uint16_t x, uint16_t y,
                          uint8_t width, uint8_t highlight) {
    uint8_t slot = id * 2 + highlight;
    cached_rectangle(7 + slot / 4, (slot & 3) * 2048, x, y, width, 36);
}

static void draw_pagination(void) {
    uint8_t state = hovered == PREVIOUS_BUTTON ? 1 : (hovered == NEXT_BUTTON ? 2 : 0);
    uint8_t slot = menu_page * 3 + state;
    cached_rectangle(11 + slot / 4, (slot & 3) * 2048, 424, 416, 41, 36);
}

static void draw_footer_control(int8_t target) {
    switch(target) {
        case HELP_BUTTON: cached_button(0, 64, 416, 22, hovered == target); break;
        case OPTIONS_BUTTON: cached_button(1, 160, 416, 34, hovered == target); break;
        case ABOUT_BUTTON: cached_button(2, 304, 416, 26, hovered == target); break;
        case PREVIOUS_BUTTON:
        case NEXT_BUTTON: draw_pagination(); break;
    }
}

static void draw_option_control(int8_t target) {
    if(target == MUSIC_BUTTON)
        cached_button(music ? 3 : 4, 228, 208, 46, hovered == target);
    else if(target == BACK_BUTTON)
        cached_button(5, 276, 248, 22, hovered == target);
}

static void draw_options(void) {
    cached_rectangle(14, 0, 176, 160, 72, 112);
    cached_rectangle(15, 0, 176, 272, 72, 32);
    draw_option_control(MUSIC_BUTTON);
    draw_option_control(BACK_BUTTON);
}

void menu_prepare_cache(void) {
    uint8_t part;
    uint8_t reload_page = cached_page != menu_page;
    uint8_t page_resource = RES_JPAGE10 + menu_page * 2;
    uint8_t details_resource = RES_JDETAIL10 + menu_page * 6;
    uint8_t old_bank = *(volatile uint8_t*)0;
    VERA.control = 0;
    if(!active) {
        *(volatile uint8_t*)0 = 7;
        resource_load(RES_JUI,BANKED_RAM,0);
        *(volatile uint8_t*)0 = 14;
        resource_load(RES_JDIALOG,BANKED_RAM,0);
        *(volatile uint8_t*)0 = old_bank;
        active = 1;
    }
    if(reload_page) {
        *(volatile uint8_t*)0 = 16;
        resource_load(page_resource++,BANKED_RAM,0);
        sound_fill_buffers();

        *(volatile uint8_t*)0 = 22;
        resource_load(page_resource++,BANKED_RAM,0);

        for(part = 0; part < 6; part++) {
            *(volatile uint8_t*)0 = 28 + part * 6;
            resource_load(details_resource+part,BANKED_RAM,0);
            sound_fill_buffers();
        }
        cached_page = menu_page;
    }
    *(volatile uint8_t*)0=old_bank;
}

void menu_init(void) {
    uint8_t i,id;
    uint8_t old_bank=*(volatile uint8_t*)0;
    transition_prepare();
    menu_prepare_cache();
    if(controls_dirty) {
        *(volatile uint8_t*)0=7;
        resource_load(RES_JRESTORE,BANKED_RAM,0);
        controls_dirty=0;
    }
    *(volatile uint8_t*)0 = old_bank;
    sound_fill_buffers();
    load_tiles(RES_JOURNAL0, 0);
    sound_fill_buffers();
    load_tiles(RES_JOURNAL1, 0xF000);
    VERA.address = 0xFC00;
    VERA.address_hi = 0x11;
    VERA.data0 = 0;
    VERA.data0 = 0x8F; /* $1E000 >> 13, plus the 8bpp sprite flag */
    VERA.layer0.config = 5;
    VERA.layer0.tilebase = 1;
    VERA.layer0.hscroll = 0;
    VERA.layer0.vscroll = 0;
    VERA.display.hscale = 128;
    VERA.display.vscale = 128;
    hovered = -1;
    pressed = -1;
    previous_buttons = 0;
    options_open = 0;
    gamestate = 0;
    for(i = 0; i < PAGE_SIZE; i++) {
        id = menu_page * PAGE_SIZE + i + 1;
        statuses[i] = retrieve_puzzle_status(id);
    }
    for(i = 0; i < PAGE_SIZE; i++) {
        draw_card(i);
        sound_fill_buffers();
    }
    draw_details();
    draw_pagination();
    transition_end(0x50);
}

void menu_leave(void) {
    transition_begin();
    controls_dirty=1;
}

static int8_t hit(uint16_t x, uint16_t y) {
    uint8_t col, row;
    uint16_t left;
    if(options_open) {
        if(y >= 208 && y < 240 && x >= 228 && x < (music ? 392 : 408)) return MUSIC_BUTTON;
        if(y >= 248 && y < 280 && x >= 276 && x < 360) return BACK_BUTTON;
        return -1;
    }
    if(y >= 112 && y < 328 && (y - 112) % 56 < 48) {
        if(x >= 100 && x < 300) {
            col = (x - 100) / 72;
        } else if(x >= 340 && x < 540) {
            col = 3 + (x - 340) / 72;
        } else return -1;
        left = card_x(col);
        if(x - left < 56) {
            row = (y - 112) / 56;
            return row * COLUMNS + col;
        }
    }
    if(y >= 416 && y < 448) {
        if(x >= 64 && x < 148) return HELP_BUTTON;
        if(x >= 160 && x < 292) return OPTIONS_BUTTON;
        if(x >= 304 && x < 404) return ABOUT_BUTTON;
        if(x >= 424 && x < 456 && menu_page) return PREVIOUS_BUTTON;
        if(x >= 552 && x < 584 && menu_page + 1 < MAX_PAGES) return NEXT_BUTTON;
    }
    return -1;
}

static void select_card(uint8_t index) {
    uint8_t old = selected;
    if(index == selected) return;
    selected = index;
    draw_card(old);
    draw_card(selected);
    draw_details();
}

static uint8_t activate(int8_t target) {
    if(target < 0) return 0;
    if(target < PAGE_SIZE) {
        play_sfx(SFX_SELECT);
        current_puzzle_id = menu_page * PAGE_SIZE + target;
        gamestate = GAME_PLAY;
        return 1;
    }
    switch(target) {
        case HELP_BUTTON: play_sfx(SFX_SELECT); gamestate = GAME_DOCVIEW_EXP; return 1;
        case ABOUT_BUTTON: play_sfx(SFX_SELECT); gamestate = GAME_DOCVIEW_ABOUT; return 1;
        case OPTIONS_BUTTON:
            play_sfx(SFX_SELECT);
            options_open = 1;
            hovered = -1;
            draw_options();
            break;
        case PREVIOUS_BUTTON: play_sfx(SFX_PAGE); menu_page--; selected = 0; menu_init(); break;
        case NEXT_BUTTON: play_sfx(SFX_PAGE); menu_page++; selected = 0; menu_init(); break;
        case MUSIC_BUTTON:
            play_sfx(SFX_SELECT);
            if(music) { stop_bgmusic(); music = NO; }
            else { music = YES; start_bgmusic(); }
            draw_option_control(MUSIC_BUTTON);
            break;
        case BACK_BUTTON: play_sfx(SFX_BACK); menu_init(); break;
    }
    return 0;
}

uint8_t menu_handle_mouse(void) {
    static uint8_t buttons;
    uint16_t* mouse_x = (uint16_t*)2;
    uint16_t* mouse_y = (uint16_t*)4;
    int8_t target;
    int8_t old_hover;
    int8_t clicked = -1;
    uint8_t key;
    uint8_t index;

    asm("ldx #2");
    asm("jsr $FF6B");
    asm("sta %v", buttons);
    target = hit(*mouse_x, *mouse_y);
    if(target != hovered) {
        old_hover = hovered;
        hovered = target;
        if(options_open) {
            draw_option_control(old_hover);
            draw_option_control(target);
        }
        else {
            if(target >= 0 && target < PAGE_SIZE) select_card(target);
            draw_footer_control(old_hover);
            draw_footer_control(target);
        }
    }
    if((buttons & 1) && !(previous_buttons & 1)) pressed = target;
    if(!(buttons & 1) && (previous_buttons & 1)) {
        if(pressed == target) clicked = target;
        pressed = -1;
    }
    previous_buttons = buttons;
    if(clicked >= 0) return activate(clicked);

    index = selected;
    key = cbm_k_getin();
    if(options_open) {
        if(key == KEYCODE_ESCAPE || key == KEYCODE_RETURN) {play_sfx(SFX_BACK);menu_init();}
    } else {
        switch(key) {
            case KEYCODE_LEFT: if(index % COLUMNS != 0) index--; break;
            case KEYCODE_RIGHT: if(index % COLUMNS != COLUMNS - 1) index++; break;
            case KEYCODE_UP: if(index >= COLUMNS) index -= COLUMNS; break;
            case KEYCODE_DOWN: if(index < PAGE_SIZE - COLUMNS) index += COLUMNS; break;
            case KEYCODE_RETURN: return activate(selected);
        }
        if(index!=selected)play_sfx(SFX_CLICK);
        select_card(index);
    }
    return 0;
}
