#include "pause.h"

// Pause state and time-bias (keeps HUD time correct)
// g_paused: 0=running, 1=paused
static unsigned char g_paused      = 0u;
// accumulated paused seconds
static unsigned int  g_pause_bias  = 0u;
// wall seconds at the moment we paused (to compute pause duration)
static unsigned int  g_pause_start = 0u;

// paused=0, bias=0, start=0
void pause_reset(void) {
    g_paused      = 0u;
    g_pause_bias  = 0u;
    g_pause_start = 0u;
}

// set paused=1, remember start=timer_seconds()
void pause_begin(void) {
    g_paused      = 1u;
    g_pause_start = timer_seconds();
}

// set paused=0, bias += (timer_seconds()-start)
void pause_end_and_account(void) {
    unsigned int now = timer_seconds();
    unsigned int dur = (unsigned int)(now - g_pause_start);
    g_pause_bias = (unsigned int)(g_pause_bias + dur);
    g_paused = 0u;
}

// 1 if currently paused, else 0
unsigned char pause_is_paused(void) {
    return g_paused;
}

// wall seconds minus total paused bias
unsigned int game_seconds(void) {
    return (unsigned int)(timer_seconds() - g_pause_bias);
}

// if paused, clear any 1s edge (to avoid instant tick on unpause)
void pause_drain_second_edge(void) {
    if (g_paused) {
        if (timer_second_edge()) { /* cleared */ }
    }
}
