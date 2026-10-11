#include <cx16.h>
#include <time.h>
#include "transition.h"
#include "sound.h"
uint8_t transition_covered=0;
static uint8_t cursor_depth;
static uint8_t scene_colors[20];
static const uint8_t colors[20]={0xC9,12,0x66,8,0x33,4,0x22,2,0xC9,12,
                                0xC9,12,0x66,8,0x33,4,0x22,2,0x66,8};
static void wait_frame(void) {
    clock_t start=clock();
    do {sound_fill_buffers();} while(clock()==start);
}
/* Six steps into paper, with an independent palette for the page indicator. */
static void palette(uint8_t step) {
    uint8_t i,v,g,b;
    VERA.control=0;
    for(i=0;i<20;i++) {
        if(i==0 || i==10) {
            VERA.address=i ? 0xFA20 : 0xFA00;VERA.address_hi=0x11;
        }
        v=scene_colors[i];
        if(i&1)v=(v*(6-step)+12*step)/6;
        else {g=((v>>4)*(6-step)+12*step)/6;b=((v&15)*(6-step)+9*step)/6;v=g*16+b;}
        VERA.data0=v;
    }
}
static void indicator(uint8_t visible) {
    uint8_t i;
    uint16_t x,address;
    VERA.control=0;
    /* Palette 15, entries 1..4; unaffected by fading the scene palettes. */
    VERA.address=0xFBE2;VERA.address_hi=0x11;
    for(i=0;i<8;i++)VERA.data0=colors[i];
    for(i=0;i<3;i++) {
        x=i==2 ? 288 : 256+i*64;
        address=(0x1F200UL+i*512UL)>>5;
        VERA.address=0xFC08+i*8;VERA.address_hi=0x11;
        VERA.data0=address;VERA.data0=address>>8;
        VERA.data0=x;VERA.data0=x>>8;
        VERA.data0=i==2 ? 212 : 240;VERA.data0=0;
        VERA.data0=visible ? 12 : 0;VERA.data0=0x7F;
    }
}
void transition_begin(void) {
    uint8_t step,i;
    if(transition_covered)return;
    VERA.control=0;
    for(i=0;i<20;i++) {
        if(i==0 || i==10) {
            VERA.address=i ? 0xFA20 : 0xFA00;VERA.address_hi=0x11;
        }
        scene_colors[i]=VERA.data0;
    }
    VERA.control=0;VERA.address=0xFC06;VERA.address_hi=0x11;
    cursor_depth=VERA.data0;
    VERA.address=0xFC06;VERA.data0=0;
    for(step=1;step<=6;step++) {palette(step);wait_frame();}
    indicator(1);
    VERA.display.video=(VERA.display.video&15)|0x50;
    transition_covered=1;
}
void transition_prepare(void) {
    transition_begin();
    VERA.display.video=(VERA.display.video&15)|0x50;
}
void transition_end(uint8_t video) {
    uint8_t step,i;
    for(i=0;i<20;i++)scene_colors[i]=colors[i];
    indicator(0);
    VERA.display.video=(VERA.display.video&15)|video;
    for(step=6;step>0;step--) {palette(step-1);wait_frame();}
    VERA.control=0;VERA.address=0xFC06;VERA.address_hi=0x11;
    VERA.data0=cursor_depth ? cursor_depth : 12;
    transition_covered=0;
}
