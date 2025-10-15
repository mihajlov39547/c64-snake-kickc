#ifndef TIMER_H
#define TIMER_H

// timer.h
// Frame/second timing system for Snake (PAL 50 Hz)
// - Can be driven manually (timer_tick) or via raster IRQ (timer_tick_irq)
// - Tracks total frames, elapsed seconds, and one-second “edge” events

// Reset all timer counters and edge flags
void timer_reset(void);

// Increment frame counter manually (call once per frame if no IRQ is used)
void timer_tick(void);

// Increment frame counter from an interrupt (IRQ-safe version of timer_tick)
void timer_tick_irq(void);

// Return total number of frames since last reset
unsigned int timer_frames(void);

// Return total number of elapsed seconds since last reset
unsigned int timer_seconds(void);

// Return 1 once per second, then clear the flag (for once-per-second events)
unsigned char timer_second_edge(void);

#endif
