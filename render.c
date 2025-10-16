// render.c
// C64 text-mode renderer for Snake:
// - Fast 40x25 screen addressing helpers
// - Full/step/grow snake drawing
// - HUD time printer (mm:ss) without division/modulo
// - Simple “Game Over” screen
// - Food drawing and playfield clear/erase

#include <c64.h>
#include "render.h"

/* --------------------------------------------------------------------
   Fast row offsets: off = row_off[y] + x  (avoids 16-bit multiply)
   MAP_W is 40 on C64 text mode.
-------------------------------------------------------------------- */
static const unsigned int row_off[MAP_H] = {
    0*MAP_W,  1*MAP_W,  2*MAP_W,  3*MAP_W,  4*MAP_W,
    5*MAP_W,  6*MAP_W,  7*MAP_W,  8*MAP_W,  9*MAP_W,
   10*MAP_W, 11*MAP_W, 12*MAP_W, 13*MAP_W, 14*MAP_W,
   15*MAP_W, 16*MAP_W, 17*MAP_W, 18*MAP_W, 19*MAP_W,
   20*MAP_W, 21*MAP_W, 22*MAP_W, 23*MAP_W, 24*MAP_W
};

/* --------------------------------------------------------------------
   Low-level helpers
-------------------------------------------------------------------- */
static inline unsigned int paddr(unsigned char x, unsigned char y) {
    return row_off[y] + x;
}

static inline void pset(unsigned char x, unsigned char y,
                        unsigned char ch, unsigned char col) {
    unsigned int off = paddr(x, y);
    SCREEN[off]    = ch;
    COLOR_RAM[off] = col;
}

static inline void pchar(unsigned char x, unsigned char y,
                         unsigned char ch, unsigned char col) {
    unsigned int off = paddr(x, y);
    SCREEN[off]    = ch;
    COLOR_RAM[off] = col;
}

/* --------------------------------------------------------------------
   Clear screen to spaces and white text.
   Also resets to the classic C64 look: light blue border (14) and blue background (6).
-------------------------------------------------------------------- */
void render_clear() {
    // Set light blue border color
    VICII->BORDER_COLOR = 14;

    // Set classic blue background color
    clear_battlefield();
}

void clear_battlefield()
{
    VICII->BG_COLOR = 6;

    // Clear 40x25: character = space, color = white
    // (two tight loops avoid 16-bit multiply in the body)
    unsigned int off = 0;
    for (unsigned char y = 0; y < MAP_H; ++y)
    {
        for (unsigned char x = 0; x < MAP_W; ++x, ++off)
        {
            SCREEN[off] = CH_EMPTY;
            COLOR_RAM[off] = COL_FG_WHITE;
        }
    }
}

/* --------------------------------------------------------------------
   Snake rendering
-------------------------------------------------------------------- */
void render_draw_snake_full(const Snake* s) {
    unsigned char i, cnt;

    // Start from the oldest segment first
    unsigned char idx = snake_tail_index(s);

    for (cnt = 0; cnt < s->len; cnt++) {
        unsigned int off = (unsigned int)s->y[idx] * MAP_W + s->x[idx];
        SCREEN[off]    = CH_SNAKE;
        COLOR_RAM[off] = COL_SNAKE;

        idx++;
        if (idx >= SNAKE_LEN) idx = 0;
    }
}

void render_apply_step(unsigned char tail_x, unsigned char tail_y,
                       unsigned char head_x, unsigned char head_y) {
    // Erase the old tail
    pset(tail_x, tail_y, CH_EMPTY, COL_FG_WHITE);

    // Draw new head
    pset(head_x, head_y, CH_SNAKE, COL_SNAKE);
}

/* --------------------------------------------------------------------
   HUD: mm:ss printer without / or % (keeps it branch-light)
-------------------------------------------------------------------- */

// Lookup table for tens digit of 0..99
static const unsigned char DIG_TENS[100] = {
  0,0,0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,3,3,
  4,4,4,4,4,4,4,4,4,4, 5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6, 7,7,7,7,7,7,7,7,7,7,
  8,8,8,8,8,8,8,8,8,8, 9,9,9,9,9,9,9,9,9,9
};

// Lookup table for ones digit of 0..99
static const unsigned char DIG_ONES[100] = {
  0,1,2,3,4,5,6,7,8,9, 0,1,2,3,4,5,6,7,8,9,
  0,1,2,3,4,5,6,7,8,9, 0,1,2,3,4,5,6,7,8,9,
  0,1,2,3,4,5,6,7,8,9, 0,1,2,3,4,5,6,7,8,9,
  0,1,2,3,4,5,6,7,8,9, 0,1,2,3,4,5,6,7,8,9
};

void render_draw_mmss_at(unsigned char x, unsigned char y, unsigned int total_seconds) {
    // Convert total_seconds to mm:ss (0..99:59) using subtract loop to avoid division
    unsigned int secs = total_seconds;
    unsigned char mm = 0;
    while (secs >= 60u) { secs -= 60u; if (mm < 99) mm++; else break; }
    unsigned char ss = (unsigned char)secs;

    unsigned char mt = DIG_TENS[mm], mo = DIG_ONES[mm];
    unsigned char st = DIG_TENS[ss], so = DIG_ONES[ss];

    pchar(x+0, y, (unsigned char)('0'+mt), COL_FG_WHITE);
    pchar(x+1, y, (unsigned char)('0'+mo), COL_FG_WHITE);
    pchar(x+2, y, ':',                       COL_FG_WHITE);
    pchar(x+3, y, (unsigned char)('0'+st), COL_FG_WHITE);
    pchar(x+4, y, (unsigned char)('0'+so), COL_FG_WHITE);
}

void render_draw_time(unsigned int total_seconds) {
    // Short, atomic HUD update (safe to call right after wait_frame()).
    render_draw_mmss_at(0, 0, total_seconds);
}

/* --------------------------------------------------------------------
   Tiny text printer (PETSCII caveats: pass screen codes or uppercase)
-------------------------------------------------------------------- */
static void print_text(unsigned char x, unsigned char y, const char* s) {
    unsigned char cx = x;
    while (*s) { pchar(cx++, y, (unsigned char)*s++, COL_FG_WHITE); }
}

/* --------------------------------------------------------------------
   Game Over screen
-------------------------------------------------------------------- */
void render_game_over(unsigned int total_seconds) {
    // Clear the screen before drawing the Game Over UI
    clear_battlefield();

    // Use a red border for a dramatic effect
    VICII->BORDER_COLOR = 2u;

    // Draw the “GAME OVER” title
    print_text(15, 11, "game over");

    // Draw the label "time " and the elapsed time in mm:ss
    print_text(15, 13, "time ");
    render_draw_mmss_at(20, 13, total_seconds);

    // Draw the restart hint
    print_text(9, 20, "press space or r to restart");
}

// Compute linear screen offset y*MAP_W + x (fits in 16-bit; KickC optimizes well)
static inline unsigned int scr_off(unsigned char x, unsigned char y) {
    return (unsigned int)y * MAP_W + x;
}

void render_draw_food(unsigned char x, unsigned char y) {
    // Place the food glyph and its color at (x,y)
    unsigned int off = scr_off(x,y);
    SCREEN[off]    = CH_FOOD;
    COLOR_RAM[off] = COL_FOOD;
}

// Draw only the new head (used when growing: no tail erase)
void render_apply_grow(unsigned char head_x, unsigned char head_y) {
    unsigned int off = scr_off(head_x, head_y);
    SCREEN[off]    = CH_SNAKE;
    COLOR_RAM[off] = COL_SNAKE;
}

// Playfield background character (space by default)
#ifndef CH_BG
#define CH_BG   0x20
#endif

// Playfield background color (C64 BLUE by default)
#ifndef COL_BG
#define COL_BG  6
#endif

// Clear the whole playfield area to the background char/color
void render_clear_playfield(void) {
    // Clear the entire playfield to the background char/color
    unsigned int off = 0;
    unsigned char y, x;
    for (y = 0; y < MAP_H; y++) {
        for (x = 0; x < MAP_W; x++) {
            // Background character for the cleared playfield (e.g., space)
            SCREEN[off]    = CH_BG;

            // Normal playfield color
            COLOR_RAM[off] = COL_BG;

            off++;
        }
    }
}

// Replace a single cell with the playfield background
void render_erase_cell(unsigned char x, unsigned char y) {
    // Replace a single cell with the playfield background
    unsigned int off = (unsigned int)y * MAP_W + x;

    // Match the cleared playfield background character
    SCREEN[off]    = CH_BG;

    // Match the cleared playfield background color
    COLOR_RAM[off] = COL_BG;
}

// Draw the "PAUSED" message centered on the screen
// (overwrites whatever was there; caller must redraw)
void render_show_pause(void) {
    // Line 1: "== PAUSED ==" (12 chars)
    static const char msg1[12] = { '=', '=', ' ', 'p','a','u','s','e','d',' ', '=', '=' };
    // Line 2: "(press SPACE to continue)" (25 chars)
    static const char msg2[25] =
        { '(', 'p','r','e','s','s',' ', 's','p','a','c','e',' ', 't','o',' ',
          'c','o','n','t','i','n','u','e', ')' };

    const unsigned char len1 = (unsigned char)12;
    const unsigned char len2 = (unsigned char)25;

    const unsigned char row1 = (unsigned char)12;
    const unsigned char row2 = (unsigned char)13;

    const unsigned char col1 = (unsigned char)((unsigned char)(40u - (unsigned int)len1) / 2u);
    const unsigned char col2 = (unsigned char)((unsigned char)(40u - (unsigned int)len2) / 2u);

    unsigned int off;
    unsigned char i;

    // Line 1
    off = (unsigned int)row1 * 40u + (unsigned int)col1;
    i = 0u;
    while (i != len1) {
        SCREEN[off + (unsigned int)i]    = (unsigned char)msg1[i];
        COLOR_RAM[off + (unsigned int)i] = (unsigned char)1;
        i++;
    }

    // Line 2
    off = (unsigned int)row2 * 40u + (unsigned int)col2;
    i = 0u;
    while (i != len2) {
        SCREEN[off + (unsigned int)i]    = (unsigned char)msg2[i];
        COLOR_RAM[off + (unsigned int)i] = (unsigned char)1;
        i++;
    }
}

// Erase the "PAUSED" message by overwriting with spaces
void render_hide_pause(void) {
    const unsigned char len1 = (unsigned char)12;
    const unsigned char len2 = (unsigned char)25;

    const unsigned char row1 = (unsigned char)12;
    const unsigned char row2 = (unsigned char)13;

    const unsigned char col1 = (unsigned char)((unsigned char)(40u - (unsigned int)len1) / 2u);
    const unsigned char col2 = (unsigned char)((unsigned char)(40u - (unsigned int)len2) / 2u);

    unsigned int off;
    unsigned char i;

    // Clear line 1
    off = (unsigned int)row1 * 40u + (unsigned int)col1;
    i = 0u;
    while (i != len1) {
        SCREEN[off + (unsigned int)i]    = (unsigned char)32; // space
        COLOR_RAM[off + (unsigned int)i] = (unsigned char)0;  // black (or keep prior)
        i++;
    }

    // Clear line 2
    off = (unsigned int)row2 * 40u + (unsigned int)col2;
    i = 0u;
    while (i != len2) {
        SCREEN[off + (unsigned int)i]    = (unsigned char)32;
        COLOR_RAM[off + (unsigned int)i] = (unsigned char)0;
        i++;
    }
}
