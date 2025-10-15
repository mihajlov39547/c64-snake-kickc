#ifndef SYS_H
#define SYS_H

#include "snake.h"

// One frame wait based on raster wrap
void wait_frame(void);

// One frame wait, timer tick, and input update
void frame_sync_and_input(Direction* pdir);

#endif
