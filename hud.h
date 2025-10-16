#ifndef HUD_H
#define HUD_H

// Ticks once per second (on timer_second_edge()).
// - Draws elapsed game time (pause-adjusted)
// - Speeds up every 10 seconds down to a limit
// - Ticks hunger (and flashes border)
// Returns 1 if hunger reached zero (starved), else 0.
unsigned char hud_tick(unsigned char* move_interval,
                       unsigned char* sec_since_speedup);

// Return 1 if (x,y) is reserved for HUD (e.g., timer area), else 0
unsigned char hud_covers_cell(unsigned char x, unsigned char y);

#endif
