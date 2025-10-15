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
    /* Center row */
    uint8_t cy = MAP_H / 2;

    /* Centered start X so the full initial length fits */
    uint8_t cx = (uint8_t)(MAP_W / 2 - (SNAKE_LEN / 2));

    s->len  = SNAKE_LEN;
    s->head = (uint8_t)(SNAKE_LEN - 1);

    /* A horizontal snake pointing right: [tail ... head] */
    for (uint8_t i = 0; i < SNAKE_LEN; ++i) {
        /* Increasing X across the row */
        s->x[i] = (uint8_t)(cx + i);
        s->y[i] = cy;
    }

    /* (Optional) initialize occupancy grid to match body */
    snake_occ_reset_from_body(s);
}

// Write head (x,y) into out parameters without modifying the snake
void snake_head_xy(const Snake* s, uint8_t* out_x, uint8_t* out_y) {
    uint8_t h = s->head;
    *out_x = s->x[h];
    *out_y = s->y[h];
}

// Compute the next wrapped head cell if moving one step in 'dir' (no mutation)
void snake_next_xy(const Snake* s, Direction dir, uint8_t* out_x, uint8_t* out_y) {
    uint8_t hx, hy;
    snake_head_xy(s, &hx, &hy);

    int8_t dx = 0, dy = 0;
    if (dir == DIR_UP)        dy = -1;
    else if (dir == DIR_DOWN) dy = +1;
    else if (dir == DIR_LEFT) dx = -1;
    else                      dx = +1;  /* DIR_RIGHT */

    *out_x = wrap_add(hx, dx, MAP_W);
    *out_y = wrap_add(hy, dy, MAP_H);
}

// Advance one step in 'dir' with wrap-around; returns the tail cell that was removed
void snake_step(Snake* s, Direction dir,
                uint8_t* out_tail_x, uint8_t* out_tail_y) {

    uint8_t head = s->head;

    /* Find the true tail index from head & len: tail = head - (len-1) wrapped on SNAKE_LEN */
    uint8_t tail = head;
    uint8_t k = (s->len > 0) ? (uint8_t)(s->len - 1) : 0;
    while (k--) {
        if (tail == 0) tail = (uint8_t)(SNAKE_LEN - 1);
        else           tail = (uint8_t)(tail - 1);
    }

    /* Current head position */
    uint8_t hx = s->x[head];
    uint8_t hy = s->y[head];

    /* Direction delta */
    int8_t dx = 0, dy = 0;
    if (dir == DIR_UP)        dy = -1;
    else if (dir == DIR_DOWN) dy = +1;
    else if (dir == DIR_LEFT) dx = -1;
    else                      dx = +1;      /* DIR_RIGHT */

    /* Next head with wrapping */
    uint8_t nx = wrap_add(hx, dx, MAP_W);
    uint8_t ny = wrap_add(hy, dy, MAP_H);

    /* Report the cell leaving the body (for renderer erase) */
    *out_tail_x = s->x[tail];
    *out_tail_y = s->y[tail];

    /* Advance ring: overwrite tail with new head and make it the new head */
    s->head    = tail;
    s->x[tail] = nx;
    s->y[tail] = ny;

    /* Keep occupancy grid consistent */
    snake_occ_clear(*out_tail_x, *out_tail_y);
    snake_occ_set(nx, ny);
}

// Advance one step in 'dir' and grow by one segment (tail is not removed)
void snake_step_grow(Snake* s, Direction dir) {
    /* Compute the next cell using the existing helper */
    uint8_t nx, ny;
    snake_next_xy(s, dir, &nx, &ny);

    /* Advance ring head */
    {
        uint8_t new_idx = s->head + 1;
        if(new_idx >= SNAKE_LEN) new_idx = 0;
        s->head = new_idx;
        s->x[new_idx] = nx;
        s->y[new_idx] = ny;
    }

    /* Mark new cell occupied; do not clear old tail */
    snake_occ_set(nx, ny);

    /* Grow length up to SNAKE_LEN */
    if(s->len < SNAKE_LEN) {
        s->len = (uint8_t)(s->len + 1);
    }
}

// Compute the current tail index: head - (len-1) modulo SNAKE_LEN
unsigned char snake_tail_index(const Snake* s) {
    /* Tail is head - (len-1) modulo SNAKE_LEN */
    unsigned char t = s->head;
    unsigned char k = (s->len > 0) ? (unsigned char)(s->len - 1) : 0;
    while (k--) {
        if (t == 0) t = (unsigned char)(SNAKE_LEN - 1);
        else        t = (unsigned char)(t - 1);
    }
    return t;
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
    unsigned char tail_i = snake_tail_index(s);
    unsigned char i;
    for (i = 0; i < SNAKE_LEN; i++) {
        // moving tail vacates its cell
        if (i == tail_i) continue;
        if (s->x[i] == nx && s->y[i] == ny) return 1u;
    }
    return 0u;
}