#ifndef HUNGER_H
#define HUNGER_H

// Seconds & UI behavior
// Hunger limit in seconds (full to empty)
#define HUNGER_LIMIT_SEC   12u
// Start flashing border when hunger <= this (seconds)
#define HUNGER_WARN_START   5u

// Colors (values per <c64.h>)
#define COL_RED           2u
#define COL_PINK         10u
#define COL_LIGHT_BLUE   14u

// Hunger API
// Call these exactly as documented by your game loop
// full hunger + calm border (call at game start)
void hunger_init(void);

// restore hunger and calm border (on eat)
void hunger_reset_on_feed(void);

// call once per timer_second_edge(); returns 1 if STARVED, else 0
unsigned char hunger_tick_and_flash(void);

// immediately set the border to the correct color for the current hunger state
void hunger_apply_border_now(void);   // set border to correct color immediately

#endif
