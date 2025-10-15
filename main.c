// main.c
// Game loop and frame pacing for C64 Snake
// - Raster-synced frame wait
// - Single-run game loop with restart-on-keypress
// - Local helpers for input, next-head computation, collision, eat handling, HUD

#include <c64.h>
#include <c64-keyboard.h>
#include "input.h"
#include "snake.h"
#include "render.h"
#include "timer.h"
#include "food.h"

// Raw VIC-II raster register accessor (simple macro; avoids casts at call sites)
#define VIC_RASTER  (*((unsigned char*)0xD012))

// Wait for the next video frame by syncing to raster wrap near the bottom
static void wait_frame(void) {
    while (VIC_RASTER != 0xFF) ;
    while (VIC_RASTER == 0xFF) ;
}

/* --------------------------------------
   Run one full game until collision.
   Returns when game over, with final time.
-------------------------------------- */
void game_loop(void) {
    Snake s;

    // Start direction is right by default
    Direction dir = DIR_RIGHT;

    // Single food item state
    Food food;

    // Initialize subsystems and draw initial state
    input_init();
    snake_init(&s);
    render_clear();
    render_draw_snake_full(&s);
    timer_reset();
    render_draw_time(0);

    // Spawn first food on a free cell and draw it
    food_init(&food, &s);

    // Timing and speed pacing (movement interval decreases over time)
    {
        unsigned char move_interval = 8;
        unsigned char frames_since_move = 0;
        unsigned char sec_since_speedup = 0;

        while (1) {
            frame_sync_and_input(&dir);

            frames_since_move++;
            if (frames_since_move >= move_interval) {
                unsigned char nx, ny;
                frames_since_move = 0;

                compute_next_head(&s, dir, &nx, &ny);

                if (self_collision_next(&s, nx, ny)) {
                    render_game_over(timer_seconds());
                    return;
                }

                if (nx == food.x && ny == food.y) {
                    handle_eat(&s, dir, &food, nx, ny);
                } else {
                    normal_step(&s, dir);
                }
            }

            hud_tick(&move_interval, &sec_since_speedup);
        }
    }
}

/* --------------------------------------
   MAIN: restart loop
-------------------------------------- */
void main(void) {
    while (1) {
        // Run a single game session
        game_loop();

        // Wait for SPACE or R to restart (still synced to frames)
        while (1) {
            keyboard_event_scan();
            if (keyboard_key_pressed(KEY_SPACE) || keyboard_key_pressed(KEY_R))
                break;
            wait_frame();
        }
    }
}

/* ---------- small helpers (local to main.c) ---------- */

// Sync one frame, tick timer, and poll/update input
static void frame_sync_and_input(Direction* pdir) {
    wait_frame();
    timer_tick();
    input_update(pdir);
}

// Compute next head coordinates with wrap-around given a direction
static void compute_next_head(const Snake* s, Direction dir,
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

// Check if the next head position (nx, ny) collides with the snake body
static unsigned char self_collision_next(const Snake* s,
                                         unsigned char nx, unsigned char ny) {
    unsigned char tail_i = snake_tail_index(s);
    unsigned char i;
    for (i = 0; i < SNAKE_LEN; i++) {
        // Skip the current tail index, since it will move away on a normal step
        if (i == tail_i) continue;
        if (s->x[i] == nx && s->y[i] == ny) return 1;
    }
    return 0;
}

// Handle eating a food item: grow snake, draw new head, erase ghost if needed, respawn food
static void handle_eat(Snake* s, Direction dir, Food* food,
                       unsigned char nx, unsigned char ny) {
    // The free slot before growth is (head+1)
    unsigned char free_i = s->head + 1;
    if (free_i >= SNAKE_LEN) free_i = 0;
    {
        unsigned char gx = s->x[free_i];
        unsigned char gy = s->y[free_i];

        // Advance head and keep tail (growth)
        snake_step_grow(s, dir);

        // Paint only the new head (no tail erase)
        render_apply_grow(nx, ny);

        // Erase the ghost cell if it is no longer part of the body after growth
        {
            unsigned char in_body = 0, cnt = 0, idx = snake_tail_index(s);
            while (cnt < s->len) {
                if (s->x[idx] == gx && s->y[idx] == gy) { in_body = 1; break; }
                idx++; if (idx >= SNAKE_LEN) idx = 0;
                cnt++;
            }
            if (!in_body) render_erase_cell(gx, gy);
        }
    }

    // Respawn food at a random free cell and draw it
    food_spawn(food, s);
    render_draw_food(food->x, food->y);
}

// Perform a normal movement step (erase tail, draw new head)
static void normal_step(Snake* s, Direction dir) {
    unsigned char old_tail_x, old_tail_y, nhx, nhy;
    snake_step(s, dir, &old_tail_x, &old_tail_y);
    snake_head_xy(s, &nhx, &nhy);
    render_apply_step(old_tail_x, old_tail_y, nhx, nhy);
}

// Update HUD once per second and speed up movement every 5 seconds down to a limit
static void hud_tick(unsigned char* move_interval,
                     unsigned char* sec_since_speedup) {
    if (timer_second_edge()) {
        unsigned int ssec = timer_seconds();
        render_draw_time(ssec);
        (*sec_since_speedup)++;
        if (*sec_since_speedup >= 5) {
            if (*move_interval > 2) (*move_interval)--;
            *sec_since_speedup = 0;
        }
    }
}
