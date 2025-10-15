#include <c64.h>
#include "sys.h"
#include "timer.h"
#include "input.h"

// Raw VIC-II raster register accessor (simple macro; avoids casts at call sites)
#define VIC_RASTER  (*((unsigned char*)0xD012))

// One frame wait based on raster wrap; also a per-frame “sync + input + timer”
void wait_frame(void) {
    while (VIC_RASTER != 0xFF) ;
    while (VIC_RASTER == 0xFF) ;
}

// One frame wait, timer tick, and input update
void frame_sync_and_input(Direction* pdir) {
    wait_frame();
    timer_tick();
    input_update(pdir);
}
