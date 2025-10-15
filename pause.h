#ifndef PAUSE_H
#define PAUSE_H

#include "timer.h"

// paused=0, bias=0, start=0
void pause_reset(void);

// set paused=1, remember start=timer_seconds()
void pause_begin(void);

// set paused=0, bias += (timer_seconds()-start)
void pause_end_and_account(void);

// 1 if currently paused, else 0
unsigned char pause_is_paused(void);

// wall seconds minus total paused bias
unsigned int game_seconds(void);

// if paused, clear any 1s edge (to avoid instant tick on unpause)
void pause_drain_second_edge(void);

#endif
