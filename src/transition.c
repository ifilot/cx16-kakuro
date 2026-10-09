#include <cx16.h>
#include <time.h>
#include "transition.h"
#include "sound.h"
uint8_t transition_covered=0;
static const uint8_t colors[20]={0xC9,12,0x66,8,0x33,4,0x22,2,0xC9,12,
                                0xC9,12,0x66,8,0x33,4,0x22,2,0x66,8};
static void wait_frame(void) {
    clock_t start=clock();
    do {sound_fill_buffers();} while(clock()==start);
}
static void palette(uint8_t paper,uint8_t halfway) {
    uint8_t i,v,g,b;
    VERA.control=0;
    for(i=0;i<20;i++) {
        if(i==0 || i==10) {
            VERA.address=i ? 0xFA20 : 0xFA00;VERA.address_hi=0x11;
        }
        v=colors[i];
        if(paper)v=(i&1) ? 12 : 0xC9;
        else if(halfway) {
            if(i&1)v=(v+12)/2;
            else {g=((v>>4)+12)/2;b=((v&15)+9)/2;v=g*16+b;}
        }
        VERA.data0=v;
    }
}
void transition_begin(void) {
    if(transition_covered)return;
    palette(0,1);
    wait_frame();palette(1,0);
    VERA.display.video=(VERA.display.video&15)|0x10;
    transition_covered=1;
}
void transition_prepare(void) {
    transition_begin();
    VERA.display.video=(VERA.display.video&15)|0x10;
}
void transition_end(uint8_t video) {
    VERA.display.video=(VERA.display.video&15)|video;
    palette(0,1);
    wait_frame();palette(0,0);
    transition_covered=0;
}
