#ifndef PLAYFIELD_H
#define PLAYFIELD_H
#include <stdint.h>
void playfield_invalidate_tiles(void);
void playfield_enter(void);
void playfield_ready(void);
void playfield_leave(void);
void playfield_cell(uint8_t row,uint8_t col);
void playfield_clue(uint8_t row,uint8_t col,uint8_t value,uint8_t down);
void playfield_controls(int8_t hover);
void playfield_clock(const char* value);
void playfield_text(const char* value,uint8_t row,uint8_t col);
void playfield_quit_modal(void);
void playfield_quit_hover(int8_t hover);
void playfield_save(void);
void playfield_restore(void);
void playfield_window(uint8_t row,uint8_t col,uint8_t height,uint8_t width);
#endif
