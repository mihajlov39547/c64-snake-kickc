#ifndef SNAKE_H
#define SNAKE_H

#include <stddef.h>
#include <stdint.h>

// Length of the snake’s ring buffer (compile-time fixed).
// If your project already defines SNAKE_LEN elsewhere, that wins.
#ifndef SNAKE_LEN
#define SNAKE_LEN 255u
#endif

// Movement directions
typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

// Snake body as a ring buffer:
// - x[i], y[i] : (screen) cell of segment i
// - head      : index of the newest segment (head) inside 0..SNAKE_LEN-1
// - len       : total segments in use (== SNAKE_LEN in this fixed-length version)
// This build starts at full length; growth calls are capped at SNAKE_LEN.
typedef struct {
    uint8_t len;
    uint8_t head;
    uint8_t x[SNAKE_LEN];
    uint8_t y[SNAKE_LEN];
} Snake;

/* ------------------------------------------------------------
   Core API (used by your game_loop)
------------------------------------------------------------ */

// Initialize the snake (centered horizontal body, pointing right)
void snake_init(Snake* s);

// Advance the snake one step in 'dir' with wrap-around.
// Writes out the tail segment that was removed (for erasing on screen).
void snake_step(Snake* s, Direction dir,
                uint8_t* out_tail_x, uint8_t* out_tail_y);

// Return the current head index inside the ring buffer
static inline uint8_t snake_head_index(const Snake* s) { return s->head; }

// Compute the next wrapped cell for the current head if we moved in 'dir'.
// This does not mutate the snake; useful for collision pre-checks before snake_step().
void snake_next_xy(const Snake* s, Direction dir, uint8_t* out_x, uint8_t* out_y);

// Rebuild the occupancy grid from the current snake body
void snake_occ_reset_from_body(const Snake* s);

// Test whether (x,y) is occupied by the snake (non-zero if occupied)
uint8_t snake_occ_test(uint8_t x, uint8_t y);

// Mark (x,y) as occupied in the occupancy grid
void snake_occ_set(uint8_t x, uint8_t y);

// Clear (x,y) from the occupancy grid
void snake_occ_clear(uint8_t x, uint8_t y);

// Advance the snake with growth (adds a segment; tail is not cleared)
void snake_step_grow(Snake* s, Direction dir);

// Compute the current tail index (head - (len-1) modulo SNAKE_LEN)
unsigned char snake_tail_index(const Snake* s);

// Check if moving to (nx,ny) would cause self-collision (non-zero if yes)
void snake_compute_next_head_wrap(const Snake* s, Direction dir, unsigned char* nx, unsigned char* ny);

// Check if (nx,ny) is occupied by the snake body excluding the tail
unsigned char snake_will_self_collide_next(const Snake* s, unsigned char nx, unsigned char ny);

// Return 1 if the snake currently occupies cell (x,y); otherwise return 0
// Performs a simple linear scan over the snake coordinates
static uint8_t snake_cell_occupied(const Snake* s, uint8_t x, uint8_t y);

#endif /* SNAKE_H */
