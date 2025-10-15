// timer.c
// Simple frame timer for PAL C64 Snake game
// - Tracks total frames and elapsed seconds
// - Supports both manual and IRQ-based ticking
// - Detects second “edge” events for once-per-second actions

#include "timer.h"
#include <c64.h>

// Frames per second (PAL = 50Hz)
#define FPS 50

// Total frame counter since last reset
static unsigned int  g_frames = 0;

// Total seconds elapsed since last reset
static unsigned int  g_sec    = 0;

// Frame counter within the current second (0..49)
static unsigned char g_insec  = 0;

// Rising-edge flag: set once when a new second starts
static unsigned char g_edge   = 0;

// Reset all timer counters and flags
void timer_reset(void) {
    g_frames = 0;
    g_sec    = 0;
    g_insec  = 0;
    g_edge   = 0;
}

// Manual frame tick
// Call once per frame if not using raster IRQ
// Increments frame count and seconds when appropriate
void timer_tick(void) {
    g_frames++;
    if (++g_insec >= FPS) {
        g_insec = 0;
        g_sec++;
        g_edge = 1;
    }
}

// IRQ-safe frame tick
// Same as timer_tick(), but safe to call from raster interrupt handler
void timer_tick_irq(void) {
    g_frames++;
    if (++g_insec >= FPS) {
        g_insec = 0;
        g_sec++;
        g_edge = 1;
    }
}

// Return total number of frames since last reset
unsigned int timer_frames(void)  { return g_frames; }

// Return total number of elapsed seconds since last reset
unsigned int timer_seconds(void) { return g_sec; }

// Return 1 once per second, then clear the edge flag
// Useful for triggering events every new second
unsigned char timer_second_edge(void) {
    unsigned char e = g_edge;
    g_edge = 0;
    return e;
}
