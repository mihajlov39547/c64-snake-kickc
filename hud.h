#ifndef HUD_H
#define HUD_H

// Ticks once per second (on timer_second_edge()).
// - Draws elapsed game time (pause-adjusted)
// - Speeds up every 5 seconds down to a limit
// - Ticks hunger (and flashes border)
// Returns 1 if hunger reached zero (starved), else 0.
unsigned char hud_tick(unsigned char* move_interval,
                       unsigned char* sec_since_speedup);

#endif
