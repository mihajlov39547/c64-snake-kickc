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
    unsigned char i;
    for (i = 0; i < s->len; ++i) {
        unsigned int off = (unsigned int)s->y[i] * MAP_W + s->x[i];
        SCREEN[off]    = CH_SNAKE;
        COLOR_RAM[off] = COL_SNAKE;
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
    // Clear the screen
    clear_battlefield();

    // Red border
    VICII->BORDER_COLOR = 2u;

    // Title & restart text (<=40 chars, uppercase)
    static const char t_title[]   = "G A M E   O V E R";
    static const char t_restart[] = "PRESS SPACE OR R TO RESTART";

    // --- Build "TIME 00:00" as one centered string ---
    // Convert total_seconds -> mm:ss with 8-bit compares only
    unsigned int secs = total_seconds;
    unsigned char mm  = 0;

    while (secs >= 60u) {
        if (mm == (unsigned char)99) {          // cap at 99 using 8-bit literal
            break;
        }
        secs -= 60u;
        mm = (unsigned char)(mm + 1);           // keep it 8-bit
    }
    unsigned char ss = (unsigned char)secs;

    // Buffer: "TIME 00:00\0" (length 10)
    char t_time[11];
    t_time[0]  = 'T';
    t_time[1]  = 'I';
    t_time[2]  = 'M';
    t_time[3]  = 'E';
    t_time[4]  = ' ';
    t_time[5]  = (char)('0' + DIG_TENS[mm]);   // tens of minutes
    t_time[6]  = (char)('0' + DIG_ONES[mm]);   // ones of minutes
    t_time[7]  = ':';
    t_time[8]  = (char)('0' + DIG_TENS[ss]);   // tens of seconds
    t_time[9]  = (char)('0' + DIG_ONES[ss]);   // ones of seconds
    t_time[10] = 0;

    // Draw
    print_centered(10, t_title,   1);  // white title
    print_centered(13, t_time,    1);  // white time line (single centered string)
    print_centered(18, t_restart, 7);  // yellow restart hint
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

// Clear a centered line by overwriting spaces (len = text length you drew)
static void clear_centered_line(unsigned char row, unsigned char len) {
    const unsigned char COLS = (unsigned char)40;
    unsigned char col = (unsigned char)(((unsigned char)(COLS - len)) >> 1);
    unsigned int off  = (unsigned int)row * 40u + (unsigned int)col;

    while (len--) {
        SCREEN[off]    = (unsigned char)32;  // space
        COLOR_RAM[off] = (unsigned char)0;   // black (or keep prior if you prefer)
        ++off;
    }
}

// Show the pause overlay (does NOT clear the whole screen)
void render_show_pause(void) {
    // Lines (<=40 chars, uppercase)
    static const char L1[] = "== PAUSED ==";                 // length 12
    static const char L2[] = "PRESS SPACE TO CONTINUE";      // length 24

    // Light blue border while paused (optional)
    VICII->BORDER_COLOR = 14u;

    // Draw centered, white
    print_centered((unsigned char)12, L1, (unsigned char)1);
    print_centered((unsigned char)13, L2, (unsigned char)1);
}

// Erase the pause overlay (restores spaces over the same area)
void render_hide_pause(void) {
    // Must match the strings used in render_show_pause()
    const unsigned char LEN1 = (unsigned char)12;  // "== PAUSED =="
    const unsigned char LEN2 = (unsigned char)24;  // "PRESS SPACE TO CONTINUE"

    clear_centered_line((unsigned char)12, LEN1);
    clear_centered_line((unsigned char)13, LEN2);
}

// ---------------------------------------------
// Title / Start screen
// ---------------------------------------------
// Show the start screen (safe for C64 UPPERCASE/GRAPHICS charset)
void render_show_start_screen(void) {
    VICII->BORDER_COLOR = 14u;
    VICII->BG_COLOR     = 6u;
    render_clear_playfield();

    print_centered(6,  "S N A K E",                       1);
    print_centered(9,  "EAT FOOD TO GROW",                1);
    print_centered(10, "EAT EVERY 12 SECONDS TO SURVIVE", 1);
    print_centered(12, "DO NOT RUN INTO YOURSELF",        1);
    print_centered(14, "MOVE WITH WASD  P FOR PAUSE",     1);
    print_centered(17, "PRESS SPACE TO START",            7);
}

// ASCII (uppercase) -> C64 screen code-
static unsigned char ascii_to_screen(unsigned char ch) {
    // Map 'A'..'Z' (65..90) -> 1..26 using 8-bit subtraction + range check
    // Other chars are mostly left as-is (space, digits, punctuation)
    unsigned char t = (unsigned char)(ch - (unsigned char)65);
    if (t <= (unsigned char)25) {
        return (unsigned char)(t + (unsigned char)1);
    }
    // Space stays space
    if (ch == (unsigned char)32) return (unsigned char)32;

    // Digits and basic punctuation are fine as-is in screen codes
    return ch;
}

// --- Centered print ---
static void print_centered(unsigned char row, const char* s, unsigned char color) {
    const unsigned char COLS = (unsigned char)40;
    unsigned char len = 0;
    const char* p = s;
    unsigned char rem = COLS;

    while ((*p != 0) && (rem != 0)) { ++len; ++p; --rem; }

    // (40 - len) / 2  using 8-bit math
    unsigned char col = (unsigned char)(((unsigned char)(COLS - len)) >> 1);

    // write chars & color
    {
        unsigned int off = (unsigned int)row * 40u + (unsigned int)col;
        unsigned char i = 0;
        while (i != len) {
            unsigned char sc = ascii_to_screen((unsigned char)s[i]);
            SCREEN[off + i]    = sc;
            COLOR_RAM[off + i] = color;
            ++i;
        }
    }
}

