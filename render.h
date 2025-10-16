#ifndef RENDER_H
#define RENDER_H

// Public interface for the C64 text-mode renderer used by Snake.
// Provides playfield addressing, colors/glyphs, and drawing/HUD routines.

#include "snake.h"

// Map dimensions for text mode
#define MAP_W 40
#define MAP_H 25

// Simple text-mode renderer base addresses
#define SCREEN      ((unsigned char*)0x0400)
#define COLOR_RAM   ((unsigned char*)0xD800)

// Foreground color used for generic text (white)
#define COL_FG_WHITE 1

// Snake color (red)
#define COL_SNAKE    2

// Empty cell character (space screen code)
#define CH_EMPTY     ' '

// Snake cell glyph (PETSCII/screencode)
#define CH_SNAKE     0xA0

// Clear entire screen to background + white foreground text
void render_clear(void);

// Clear battlefield only
void clear_battlefield();

// Draw the entire snake from its ring buffer (full redraw)
void render_draw_snake_full(const Snake* s);

// Apply one movement step: erase tail cell and draw head cell
void render_apply_step(unsigned char tail_x, unsigned char tail_y,
                       unsigned char head_x, unsigned char head_y);

// Draw the HUD time at a fixed position (0,0) as mm:ss
void render_draw_time(unsigned int total_seconds);

// Draw the HUD time at (x,y) as mm:ss without division/modulo
void render_draw_mmss_at(unsigned char x, unsigned char y, unsigned int total_seconds);

// Draw the Game Over screen with elapsed time and restart hint
void render_game_over(unsigned int total_seconds);

// Food color (C64 YELLOW). Kept consistent with the rest of the palette.
#define COL_FOOD     7

// Food glyph (distinct PETSCII/screencode)
#define CH_FOOD      0x51

// Draw a food glyph at (x,y)
void render_draw_food(unsigned char x, unsigned char y);

// Draw only the new snake head (used when growing; no tail erase)
void render_apply_grow(unsigned char head_x, unsigned char head_y);

// Clear the whole playfield area to the background char/color
void render_clear_playfield(void);

// Replace a single cell with the playfield background
void render_erase_cell(unsigned char x, unsigned char y);

// Show or hide the "PAUSED" message at fixed position (16,12)
void render_show_pause(void);
void render_hide_pause(void);


#endif
