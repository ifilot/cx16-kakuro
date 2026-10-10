#include "resource.h"
/* Four-color journal pages. Scrolling updates only the 8px text overlay. */
#include "docview.h"
#include "transition.h"
#include "playfield.h"
#define DOC_MAP 0x18000UL
#define VISIBLE 18
uint16_t docview_line_count, docview_top_line;
static uint8_t pressed;
static int8_t hover=-1, pressed_target=-1;
extern const uint8_t* menu_blit_source;
extern uint8_t menu_blit_width,menu_blit_height;
extern void menu_blit(void);
extern void docview_text_row(void),docview_bar(void);
extern uint8_t docview_color;
extern uint16_t docview_thumb_top,docview_thumb_end;

static void address(uint32_t value) {
    VERA.control=0; VERA.address=value; VERA.address_hi=0x10|(value>>16);
}
static void load(uint8_t id,uint8_t bank) {
    *(volatile uint8_t*)0=bank;
    resource_load(id,BANKED_RAM,0);
}
static uint16_t maximum(void) {return docview_line_count>VISIBLE ? docview_line_count-VISIBLE : 0;}
static void control(uint8_t id) {
    uint8_t state,slot,bank=*(volatile uint8_t*)0;
    state=hover==id ? 1 : 0;
    if((id==0 && !docview_top_line)||(id==1 && docview_top_line==maximum()))state=2;
    slot=id*3+state;
    *(volatile uint8_t*)0=7+slot/8;
    menu_blit_source=(const uint8_t*)(BANKED_RAM+(slot&7)*1024);
    menu_blit_width=16;menu_blit_height=24;
    address(432*160UL+(id==0 ? 440 : 520)/4);
    menu_blit();*(volatile uint8_t*)0=bank;
}
void docview_render(void) {
    uint8_t row,col,bank=*(volatile uint8_t*)0,color;
    uint16_t line,thumb,top;
    const uint8_t* record;
    char status[]="LINES 000-000 OF 000";
    uint16_t numbers[3];
    static const char blank[60]="                                                            ";
    *(volatile uint8_t*)0=6;
    for(row=0;row<VISIBLE;row++) {
        line=docview_top_line+row;
        record=(const uint8_t*)(BANKED_RAM+2+line*64);
        color=line<docview_line_count ? record[60] : 0;
        address(DOC_MAP+(14+row*2)*256UL+18);
        menu_blit_source=line<docview_line_count ? record : (const uint8_t*)blank;
        docview_color=color;docview_text_row();
    }
    *(volatile uint8_t*)0=bank;
    numbers[0]=docview_top_line+1;
    numbers[1]=docview_top_line+VISIBLE<docview_line_count ? docview_top_line+VISIBLE : docview_line_count;
    numbers[2]=docview_line_count;
    for(col=0;col<3;col++) {
        row=col==0 ? 6 : col==1 ? 10 : 17;
        status[row]='0'+numbers[col]/100;
        status[row+1]='0'+numbers[col]/10%10;
        status[row+2]='0'+numbers[col]%10;
    }
    address(DOC_MAP+50*256UL+18);
    for(col=0;status[col];col++) {VERA.data0=status[col]-32;VERA.data0=1;}
    thumb=maximum() ? VISIBLE*288/docview_line_count : 288;
    if(thumb<16)thumb=16;
    top=maximum() ? docview_top_line*(288-thumb)/maximum() : 0;
    docview_thumb_top=top;docview_thumb_end=top+thumb;
    address(0);docview_bar();
    control(0);control(1);
}
static void scroll(int16_t amount) {
    int16_t next=docview_top_line+amount;
    if(next<0)next=0;
    if(next>maximum())next=maximum();
    if(next!=docview_top_line) {play_sfx(amount==1 || amount==-1 ? SFX_CLICK : SFX_PAGE);docview_top_line=next;docview_render();}
}
void docview_init_screen(void) {
    uint8_t bank=*(volatile uint8_t*)0;
    transition_prepare();
    load_tiles(gamestate==GAME_DOCVIEW_EXP ? RES_DHELP0 : RES_DABOUT0,0);
    sound_fill_buffers();
    load_tiles(gamestate==GAME_DOCVIEW_EXP ? RES_DHELP1 : RES_DABOUT1,0xF000);
    playfield_invalidate_tiles();
    load_tiles(RES_FONT8,0x13000UL);
    load(RES_DCONTROL,7);*(volatile uint8_t*)0=bank;
    address(DOC_MAP);transfer_count=16384;transfer_clear();
    VERA.layer0.config=5;VERA.layer0.tilebase=1;
    VERA.layer0.hscroll=0;VERA.layer0.vscroll=0;
    VERA.layer1.config=0x68;VERA.layer1.tilebase=0x13000UL>>9;VERA.layer1.mapbase=DOC_MAP>>9;
    VERA.layer1.hscroll=0;VERA.layer1.vscroll=0;
    VERA.display.hscale=128;VERA.display.vscale=128;
    address(0x1FC00UL);VERA.data0=0;VERA.data0=0x8F;
    hover=-1;pressed=0;pressed_target=-1;
}
void docview_load_text(uint8_t resource) {
    uint8_t bank=*(volatile uint8_t*)0;
    load(resource,6);
    docview_line_count=*(uint16_t*)BANKED_RAM;docview_top_line=0;
    *(volatile uint8_t*)0=bank;
}
void docview_show_file(void) {docview_render();transition_end(0x70);}
void docview_leave(void) {transition_begin();}
void docview_handle_key(void) {
    static uint8_t key,buttons;
    static int8_t wheel;
    uint16_t x,y;
    int8_t next=-1,old;
    asm("jsr $FFE4");asm("sta %v",key);
    if(key==KEYCODE_ESCAPE) {play_sfx(SFX_BACK);gamestate=0;return;}
    if(key==0x91)scroll(-1);
    if(key==0x11)scroll(1);
    if(key==0x82)scroll(-VISIBLE);
    if(key==0x02 || key==' ')scroll(VISIBLE);
    if(key==0x13)scroll(-127);
    if(key==0x04)scroll(127);
    asm("ldx #2");asm("jsr $FF6B");asm("sta %v",buttons);asm("stx %v",wheel);
    x=*(uint16_t*)2;y=*(uint16_t*)4;
    if(wheel)scroll(wheel*3);
    if(y>=432 && y<456) {
        if(x>=440 && x<504)next=0;
        if(x>=520 && x<584)next=1;
    }
    if(next!=hover) {old=hover;hover=next;if(old>=0)control(old);if(next>=0)control(next);}
    if((buttons&1) && !pressed)pressed_target=hover;
    if(!(buttons&1) && pressed && pressed_target==hover) {
        if(hover==0)scroll(-1);
        if(hover==1)scroll(1);
        if(x>=572 && x<588 && y>=112 && y<400) {
            if(y<112+docview_thumb_top)scroll(-VISIBLE);
            else if(y>=112+docview_thumb_end)scroll(VISIBLE);
        }
    }
    pressed=buttons&1;
}
