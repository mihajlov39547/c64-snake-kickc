#include "render.h"
#include "timer.h"
#include "pause.h"
#include "hunger.h"

// Ticks once per second (on timer_second_edge()).
// - Draws elapsed game time (pause-adjusted)
// - Speeds up every 10 seconds down to a limit
// - Ticks hunger (and flashes border)
// Returns 1 if hunger reached zero (starved), else 0.
unsigned char hud_tick(unsigned char* move_interval,
                       unsigned char* sec_since_speedup) {
    // If paused, drain the edge and do nothing this frame
    if (pause_is_paused()) {
        pause_drain_second_edge();
        return 0u;
    }

    if (timer_second_edge()) {
        // Time HUD (pause-adjusted)
        render_draw_time(game_seconds());

        // Speed ramp (every 10 seconds, to a floor of 2)
        (*sec_since_speedup)++;
        if ((unsigned char)(*sec_since_speedup) >= (unsigned char)10) {
            if ((unsigned char)(*move_interval) > (unsigned char)2) {
                (*move_interval)--;
            }
            *sec_since_speedup = 0u;
        }

        // Hunger countdown + border flash
        if (hunger_tick_and_flash()) {
            return 1u;  // starved -> game over
        }
    }
    return 0u;
}

static unsigned char g_timer_x = 0u;
static unsigned char g_timer_y = 0u;

// returns 1 if (x,y) is inside "MM:SS" (5 chars wide on one row)
unsigned char hud_covers_cell(unsigned char x, unsigned char y) {
    if (y == g_timer_y) {
        if (x >= g_timer_x && x <= (unsigned char)(g_timer_x + 4u)) return 1u;
    }
    return 0u;
}