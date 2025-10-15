// food.c
// - SID-based RNG + food spawning helper for Snake
// - RNG uses SID voice 3 and initializes on first use
// - Food is spawned only on unoccupied cells
// - All comments live on their own line above the code they explain

#include <c64.h>
#include "food.h"

// Tracks whether SID RNG has been initialized (0 = no, 1 = yes)
static unsigned char g_rng_inited = 0;

// Return a random 8-bit value using SID voice 3
// Initializes the SID RNG once on first call
uint8_t rng8(void) {
    if(!g_rng_inited) {
        // One-time init of SID voice 3 RNG
        sid_rnd_init();
        g_rng_inited = 1;
    }
    // Fetch a random byte from SID
    return (uint8_t)sid_rnd();
}

// Reduce v into the range [0 .. limit-1] using repeated subtraction
// This avoids division/modulo and is fragment-friendly
static uint8_t wrap_under(uint8_t v, uint8_t limit) {
    while (v >= limit) v -= limit;
    return v;
}

// Return 1 if the snake currently occupies cell (x,y); otherwise return 0
// Performs a simple linear scan over the snake coordinates
static uint8_t snake_cell_occupied(const Snake* s, uint8_t x, uint8_t y) {
    uint8_t i = 0;
    uint8_t len = s->len;
    for (i = 0; i < len; i++) {
        if (s->x[i] == x && s->y[i] == y) return 1;
    }
    return 0;
}

// Pick a random free cell and store it into f->x/f->y
// Re-rolls until a cell not occupied by the snake is found
static void spawn_once(Food* f, const Snake* s) {
    uint8_t x;
    uint8_t y;
    do {
        x = wrap_under(rng8(), MAP_W);
        y = wrap_under(rng8(), MAP_H);
    } while (snake_cell_occupied(s, x, y));
    f->x = x;
    f->y = y;
}

// Respawn food at a new free cell (does not draw it)
void food_spawn(Food* f, const Snake* s) {
    spawn_once(f, s);
}

// Initialize RNG stirring + spawn first food, then draw it
// Discards a handful of RNG bytes so first values differ across runs
void food_init(Food* f, const Snake* s) {
    unsigned char i;

    // Stir the RNG a bit to decorrelate initial state across resets
    for(i = 0; i < 16; i++) rng8();

    // Choose a free cell
    spawn_once(f, s);

    // Draw the newly spawned food
    render_draw_food(f->x, f->y);
}
