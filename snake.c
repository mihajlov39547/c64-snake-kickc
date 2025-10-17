#include "snake.h"

// For MAP_W / MAP_H
#include "render.h"
#include <string.h>
#include <stdint.h>

/* ------------------------------------------------------------
   Internal: wrap helpers for 0..(lim-1)  (d is -1, 0, or +1)
------------------------------------------------------------ */

// Add -1/0/+1 to v with wrap-around in [0..lim-1]
static uint8_t wrap_add(uint8_t v, int8_t d, uint8_t lim) {
    if (d > 0) {
        if (++v >= lim) v = 0;
    } else if (d < 0) {
        if (v == 0) v = (uint8_t)(lim - 1);
        else --v;
    }
    return v;
}

/* ------------------------------------------------------------
   Optional occupancy grid (bitset W*H)
   NOTE: Avoid '~' operator (KickC 0.8.6 fragment gap) by using tables.
------------------------------------------------------------ */
#define OCC_W   (MAP_W)
#define OCC_H   (MAP_H)
#define OCC_SZ  ((OCC_W*OCC_H + 7u)/8u)

static uint8_t g_occ[OCC_SZ];

/* Bit tables for set/clear/test without '~' */
static const uint8_t BIT[8]   = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80 };
static const uint8_t NBIT[8]  = { 0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F };

// Compute linear bit index into occupancy grid for (x,y)
static inline uint16_t occ_idx(uint8_t x, uint8_t y) { return (uint16_t)y * OCC_W + x; }

// Mark a cell as occupied in the bitset using precalculated bit masks
static inline void     occ_set(uint16_t i)  { uint8_t b = (uint8_t)(i & 7); g_occ[i >> 3] |=  BIT[b]; }

// Clear a cell from the bitset using precalculated inverse masks
static inline void     occ_clr(uint16_t i)  { uint8_t b = (uint8_t)(i & 7); g_occ[i >> 3] &= NBIT[b]; }

// Test whether a cell is occupied (returns non-zero if set)
static inline uint8_t  occ_get(uint16_t i)  { uint8_t b = (uint8_t)(i & 7); return (uint8_t)(g_occ[i >> 3] & BIT[b]); }

/* Public wrappers: rebuild occupancy from snake body */

// Rebuild the occupancy grid from the current snake body
void snake_occ_reset_from_body(const Snake* s) {
    memset(g_occ, 0, sizeof(g_occ));
    for (uint8_t i = 0; i < s->len; ++i) {
        occ_set(occ_idx(s->x[i], s->y[i]));
    }
}

// Check if coordinate (x,y) is occupied by the snake (non-zero if yes)
uint8_t snake_occ_test(uint8_t x, uint8_t y) { return occ_get(occ_idx(x, y)); }

// Mark coordinate (x,y) as occupied
void    snake_occ_set (uint8_t x, uint8_t y) { occ_set(occ_idx(x, y)); }

// Mark coordinate (x,y) as free
void    snake_occ_clear(uint8_t x, uint8_t y) { occ_clr(occ_idx(x, y)); }

/* ------------------------------------------------------------
   Core API
------------------------------------------------------------ */

// Initialize snake as a centered horizontal line pointing right; seed occupancy grid
void snake_init(Snake* s) {
    // Initial length and centered position
    const uint8_t start_len = 16;
    const uint8_t cy = (uint8_t)(MAP_H / 2);
    const uint8_t cx = (uint8_t)(MAP_W / 2 - (start_len / 2));

    // Set length and head index
    s->len  = start_len;
    s->head = (uint8_t)(start_len - 1);

    // Fill contiguous body: indices 0..len-1 == tail..head
    for (uint8_t i = 0; i < start_len; ++i) {
        s->x[i] = (uint8_t)(cx + i);
        s->y[i] = cy;
    }

    // seed occupancy from the active body
    snake_occ_reset_from_body(s);
}

// Write head (x,y) into out parameters without modifying the snake
void snake_head_xy(const Snake* s, uint8_t* out_x, uint8_t* out_y) {
    uint8_t h = (s->len > 0) ? (uint8_t)(s->len - 1) : 0;
    *out_x = s->x[h];
    *out_y = s->y[h];
}

// Compute the next wrapped head cell if moving one step in 'dir' (no mutation)
void snake_next_xy(const Snake* s, Direction dir, uint8_t* out_x, uint8_t* out_y) {
    uint8_t hx, hy;
    snake_head_xy(s, &hx, &hy);

    // Direction delta
    int8_t dx = 0, dy = 0;
    if (dir == DIR_UP)        dy = -1;
    else if (dir == DIR_DOWN) dy = +1;
    else if (dir == DIR_LEFT) dx = -1;
    else                      dx = +1;

    // Next head with wrapping
    *out_x = wrap_add(hx, dx, MAP_W);
    *out_y = wrap_add(hy, dy, MAP_H);
}

// Advance one step in 'dir' with wrap-around; returns the tail cell that was removed
void snake_step(Snake* s, Direction dir,
                uint8_t* out_tail_x, uint8_t* out_tail_y) {

    uint8_t nx, ny;
    snake_next_xy(s, dir, &nx, &ny);

    *out_tail_x = s->x[0];
    *out_tail_y = s->y[0];

    // shift body: [1]->[0], ..., [len-1]->[len-2]
    uint8_t len = s->len;
    for (uint8_t i = 0; i + 1 < len; ++i) {
        s->x[i] = s->x[i+1];
        s->y[i] = s->y[i+1];
    }

    // write new head
    s->x[len-1] = nx;
    s->y[len-1] = ny;
    s->head = (uint8_t)(len - 1);

    snake_occ_clear(*out_tail_x, *out_tail_y);
    snake_occ_set(nx, ny);
}

// Advance one step in 'dir' and grow by one segment (tail is not removed)
// Grow by one segment (no tail removal). If already at SNAKE_LEN, behave like a normal step.
void snake_step_grow(Snake* s, Direction dir) {
    uint8_t nx, ny;
    snake_next_xy(s, dir, &nx, &ny);

    // Still room? append at the first free slot (index == len)
    if (s->len < (uint8_t)SNAKE_LEN) {
        // Append new segment
        uint8_t idx = s->len;
        s->x[idx] = nx;
        s->y[idx] = ny;
        // Advance logical length and head
        s->len = (uint8_t)(idx + 1);
        s->head = (uint8_t)(s->len - 1);
        // Mark new head as occupied
        snake_occ_set(nx, ny);
    } else {
        // At capacity: just do a normal step so gameplay continues
        uint8_t tx, ty;
        snake_step(s, dir, &tx, &ty);
    }
}

unsigned char snake_tail_index(const Snake* s) {
    return 0u;
}

// Compute the next wrapped head cell if moving in 'dir' (no mutation)
void snake_compute_next_head_wrap(const Snake* s, Direction dir,
                                  unsigned char* nx, unsigned char* ny) {
    unsigned char hx, hy;
    signed char dx = 0, dy = 0;
    snake_head_xy(s, &hx, &hy);

    if (dir==DIR_UP) dy = -1;
    else if (dir==DIR_DOWN) dy = +1;
    else if (dir==DIR_LEFT) dx = -1;
    else dx = +1;

    // X wrap
    *nx = hx;
    if (dx > 0) { (*nx)++; if (*nx >= MAP_W) *nx = 0; }
    else if (dx < 0) { if (*nx == 0) *nx = (unsigned char)(MAP_W-1); else (*nx)--; }

    // Y wrap
    *ny = hy;
    if (dy > 0) { (*ny)++; if (*ny >= MAP_H) *ny = 0; }
    else if (dy < 0) { if (*ny == 0) *ny = (unsigned char)(MAP_H-1); else (*ny)--; }
}

// Test if moving to (nx,ny) would collide with the snake body (excluding moving tail)
unsigned char snake_will_self_collide_next(const Snake* s,
                                           unsigned char nx, unsigned char ny) {
    for (uint8_t i = 1; i < s->len; ++i) {
        if (s->x[i] == nx && s->y[i] == ny) return 1u;
    }
    return 0u;
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