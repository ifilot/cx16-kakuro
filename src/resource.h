#ifndef RESOURCE_H
#define RESOURCE_H
#include <stdint.h>
#include "resource_index.h"

uint8_t resource_init(void);
uint8_t resource_load(uint8_t id, uint16_t destination, uint8_t stream);
uint8_t resource_vram(uint8_t id, uint32_t destination);
extern uint16_t resource_pointer, resource_remaining;
extern uint8_t resource_stream, resource_banked, resource_error;
void resource_read(void);
#endif
