#ifndef TRANSITION_H
#define TRANSITION_H
#include <stdint.h>
/* Fade through paper with a persistent page indicator while scenes load. */
void transition_begin(void);
void transition_end(uint8_t video);
void transition_prepare(void);
extern uint8_t transition_covered;
extern uint16_t transfer_count;
extern uint8_t* transfer_pointer;
void transfer_clear(void);
void transfer_read(void);
void transfer_write(void);
void rectangle_read(void);
void rectangle_write(void);
#endif
