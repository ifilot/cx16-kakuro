/* Journal playfield: 2bpp scenery underneath a 4bpp tile overlay.
 * Board changes touch four tile-map entries; no bitmap redraw is needed. */
#include "playfield.h"
#include "puzzle.h"
#include "transition.h"

#define GRAPHICS 0x13000UL
#define GAME_MAP 0x1E200UL
#define FONT_TILE 285
#define BORDER_TILE 344
extern uint8_t* userdata;
extern const uint8_t* menu_blit_source;
extern uint8_t menu_blit_width,menu_blit_height;
extern void menu_blit(void);
static uint8_t tiles_loaded=0;
void playfield_invalidate_tiles(void) {tiles_loaded=0;}
static int8_t old_hover = -1;
static uint8_t old_verify = 255;

static void map_tile(uint8_t row,uint8_t col,uint16_t tile,uint8_t flags) {
    VERA.control=0;
    VERA.address=GAME_MAP+(row*64+col)*2;
    VERA.address_hi=0x11;
    VERA.data0=tile;
    VERA.data0=(tile>>8)|flags;
}

void playfield_text(const char* value,uint8_t row,uint8_t col) {
    uint8_t c;
    while(*value) {
        c=*value++;
        if(c>=97 && c<=122)c-=32;
        if(c<32 || c>90)c=32;
        map_tile(row,col++,FONT_TILE+c-32,0);
    }
}

static void control(uint8_t id,uint8_t highlight) {
    uint8_t slot=id*2+highlight;
    uint8_t bank=*(volatile uint8_t*)0;
    *(volatile uint8_t*)0=7+slot/4;
    menu_blit_source=(const uint8_t*)(BANKED_RAM+(slot&3)*2048);
    menu_blit_width=42;
    menu_blit_height=36;
    VERA.control=0;
    VERA.address=288*160UL+436/4;
    VERA.address_hi=0x10;
    menu_blit();
    *(volatile uint8_t*)0=bank;
}

void playfield_controls(int8_t hover) {
    uint8_t verify=!!(gamestate&GAME_VERIFY);
    if(hover==old_hover && verify==old_verify)return;
    control(verify ? 1 : 0,hover==1);
    old_hover=hover;old_verify=verify;
}

void playfield_enter(void) {
    uint8_t i,bank=*(volatile uint8_t*)0;
    uint8_t status;
    char label[]="NO. 000";
    char shape[]="0X0";
    uint8_t id=current_puzzle_id+1;
    transition_prepare();
    load_tiles("gplay0.dat",0);
    sound_fill_buffers();
    load_tiles("gplay1.dat",0xF000);
    if(!tiles_loaded) {load_tiles("gtiles.dat",GRAPHICS);tiles_loaded=1;}
    *(volatile uint8_t*)0=7;
    cbm_k_setnam("gcontrols.dat");cbm_k_setlfs(0,8,2);cbm_k_load(0,BANKED_RAM);
    *(volatile uint8_t*)0=bank;
    VERA.control=0;VERA.address=0xE200;VERA.address_hi=0x11;
    transfer_count=4096;transfer_clear();
    VERA.layer0.config=5;VERA.layer0.tilebase=1;
    VERA.layer0.hscroll=0;VERA.layer0.vscroll=0;
    VERA.layer1.config=0x12;
    VERA.layer1.tilebase=(GRAPHICS>>9)|3;
    VERA.layer1.mapbase=GAME_MAP>>9;
    VERA.layer1.hscroll=0;VERA.layer1.vscroll=0;
    VERA.display.hscale=128;VERA.display.vscale=128;
    VERA.address=0xFC00;VERA.address_hi=0x11;
    VERA.data0=0;VERA.data0=0x8F;
    label[4]='0'+id/100;label[5]='0'+id/10%10;label[6]='0'+id%10;
    playfield_text(label,6,28);
    shape[0]='0'+puzzlecols;shape[2]='0'+puzzlerows;
    if(puzzlecols<10)playfield_text(shape,8,28);
    else playfield_text("10X10",8,28);
    status=retrieve_puzzle_status(id);
    for(i=0;i<5;i++)map_tile(11,28+i,i<((status>>6)+1) ? 349 : 350,0);
    old_hover=-1;old_verify=255;
    playfield_controls(-1);
}

void playfield_ready(void) {transition_end(0x70);}

void playfield_leave(void) {transition_begin();}

void playfield_cell(uint8_t row,uint8_t col) {
    uint8_t index=row*puzzlecols+col;
    uint8_t value=userdata[index]&15;
    uint8_t data=puzzledata[index];
    uint8_t type,flags=0;
    uint16_t tile;
    if(!(data&15))type=(data&(TLDT_HCLUE|TLDT_VCLUE)) ? 1 : 0;
    else if(data&TLDT_REVEALED)type=21+value;
    else if(ccurx==col && ccury==row) {
        type=12+value;
        if(value && (gamestate&GAME_VERIFY) && value!=(data&15))flags=0x10;
    } else if(!value)type=2;
    else if(gamestate&GAME_VERIFY)type=(value==(data&15) ? 30 : 39)+value;
    else type=2+value;
    tile=1+type*4;
    map_tile(offset_y+row*2,offset_x+col*2,tile,flags);
    map_tile(offset_y+row*2,offset_x+col*2+1,tile+1,flags);
    map_tile(offset_y+row*2+1,offset_x+col*2,tile+2,flags);
    map_tile(offset_y+row*2+1,offset_x+col*2+1,tile+3,flags);
}

void playfield_clue(uint8_t row,uint8_t col,uint8_t value,uint8_t down) {
    map_tile(offset_y+row*2+down,offset_x+col*2+!down,197+(value-2)*2+down,0);
}

void playfield_clock(const char* value) {playfield_text(value,15,28);}

void playfield_window(uint8_t row,uint8_t col,uint8_t height,uint8_t width) {
    uint8_t x,y;
    for(y=row-1;y<=row+height;y++)for(x=col-1;x<=col+width;x++) {
        uint16_t tile=FONT_TILE;
        uint8_t flags=0;
        if(y==row-1)tile=BORDER_TILE;
        if(y==row+height)tile=BORDER_TILE+3;
        if(x==col-1 || x==col+width) {
            if(y==row-1)tile=BORDER_TILE+2;
            else if(y==row+height)tile=BORDER_TILE+4;
            else tile=BORDER_TILE+1;
            if(x==col+width)flags=MIRROR_X;
        }
        map_tile(y,x,tile,flags);
    }
}

/* Save the 64x32 overlay in lower bank 5, and the 384x96 modal background
 * in upper bank 5 plus bank 6. The atlas and persistent modal stay intact. */
void playfield_save(void) {
    uint8_t bank=*(volatile uint8_t*)0;
    sound_fill_buffers();
    *(volatile uint8_t*)0=RAMBANK_SCREEN;
    VERA.control=0;VERA.address=0xE200;VERA.address_hi=0x11;
    transfer_pointer=(uint8_t*)BANKED_RAM;transfer_count=4096;transfer_read();
    menu_blit_source=(const uint8_t*)(BANKED_RAM+4096);
    menu_blit_width=96;menu_blit_height=96;
    VERA.address=192*160UL+32;VERA.address_hi=0x10;
    rectangle_read();
    *(volatile uint8_t*)0=bank;
}

void playfield_restore(void) {
    uint8_t bank=*(volatile uint8_t*)0;
    *(volatile uint8_t*)0=RAMBANK_SCREEN;
    VERA.control=0;VERA.address=0xE200;VERA.address_hi=0x11;
    transfer_pointer=(uint8_t*)BANKED_RAM;transfer_count=4096;transfer_write();
    menu_blit_source=(const uint8_t*)(BANKED_RAM+4096);
    menu_blit_width=96;menu_blit_height=96;
    VERA.address=192*160UL+32;VERA.address_hi=0x10;
    rectangle_write();
    *(volatile uint8_t*)0=bank;
}

void playfield_quit_hover(int8_t hover) {
    uint8_t id,bank=*(volatile uint8_t*)0;
    *(volatile uint8_t*)0=4;
    for(id=0;id<2;id++) {
        menu_blit_source=(const uint8_t*)(BANKED_RAM+1024+(id*2+(hover==id))*640);
        menu_blit_width=32;menu_blit_height=20;
        VERA.control=0;VERA.address=256*160UL+(id ? 344 : 168)/4;VERA.address_hi=0x10;
        menu_blit();
    }
    *(volatile uint8_t*)0=bank;
}
void playfield_quit_modal(void) {
    uint8_t row,col,bank=*(volatile uint8_t*)0;
    for(row=12;row<18;row++)for(col=8;col<32;col++)map_tile(row,col,0,0);
    *(volatile uint8_t*)0=3;
    menu_blit_source=(const uint8_t*)BANKED_RAM;
    menu_blit_width=96;menu_blit_height=96;
    VERA.control=0;VERA.address=192*160UL+32;VERA.address_hi=0x10;
    rectangle_write();
    *(volatile uint8_t*)0=bank;
}
