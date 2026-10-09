/* A persistent channel and DOS byte-offset seeks keep disk layout independent
 * of the existing RAM banks and VERA locations. Music uses separate channels. */
#include <cbm.h>
#include <cx16.h>
#define RESOURCE_IMPLEMENTATION
#include "resource.h"
#define RESOURCE_CHANNEL 14

uint16_t resource_pointer, resource_remaining;
uint8_t resource_stream, resource_banked, resource_error;

uint8_t resource_init(void) {
    uint8_t header[12];
    if(cbm_open(RESOURCE_CHANNEL,8,RESOURCE_CHANNEL,"assets.dat,s,r"))return 0;
    if(cbm_read(RESOURCE_CHANNEL,header,sizeof(header))!=sizeof(header)
       || header[0]!=0x4b || header[1]!=0x4b || header[2]!=0x52 || header[3]!=0x31 || header[4]!=RESOURCE_COUNT
       || header[5] || header[6] || header[7]
       || header[8]!=RESOURCE_LAYOUT_0 || header[9]!=RESOURCE_LAYOUT_1
       || header[10]!=RESOURCE_LAYOUT_2 || header[11]!=RESOURCE_LAYOUT_3) {
        cbm_close(RESOURCE_CHANNEL);return 0;
    }
    return 1;
}

uint8_t resource_load(uint8_t id,uint16_t destination,uint8_t stream) {
    uint8_t command[6],bank=*(volatile uint8_t*)0;
    uint32_t offset;
    if(id>=RESOURCE_COUNT)return 0;
    offset=resource_index[id].offset;
    command[0]=0x50;command[1]=RESOURCE_CHANNEL;
    command[2]=offset;command[3]=offset>>8;
    command[4]=offset>>16;command[5]=offset>>24;
    if(cbm_open(15,8,15,""))return 0;
    if(cbm_write(15,command,sizeof(command))!=sizeof(command)) {
        cbm_close(15);return 0;
    }
    cbm_close(15);
    if(cbm_k_chkin(RESOURCE_CHANNEL))return 0;
    resource_pointer=destination;resource_remaining=resource_index[id].size;
    resource_stream=stream;
    resource_banked=!stream && destination>=0xA000 && destination<0xC000;
    resource_error=0;resource_read();
    cbm_k_clrch();*(volatile uint8_t*)0=bank;
    return !resource_error;
}

uint8_t resource_vram(uint8_t id,uint32_t destination) {
    VERA.control=0;VERA.address=destination;
    VERA.address_hi=0x10|(destination>>16);
    return resource_load(id,0x9F23,1);
}
