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
#include "sys.h"
#include "hud.h"
#include "hunger.h"
#include "pause.h"

// Raw VIC-II raster register accessor (simple macro; avoids casts at call sites)
#define VIC_RASTER  (*((unsigned char*)0xD012))

// --- Hunger countdown (seconds) ---
#define HUNGER_LIMIT_SEC      12u
#define HUNGER_WARN_START      5u

// C64 palette colors (values as per <c64.h>)
#define COL_RED           2u
#define COL_PINK         10u
#define COL_LIGHT_BLUE   14u
#define COL_YELLOW       7u

/* --------------------------------------
   Run one full game until collision.
   Returns when game over, with final time.
-------------------------------------- */
static void game_loop(void) {
    Snake s;
    Direction dir = DIR_RIGHT;
    Food food;

    input_init();
    snake_init(&s);
    render_clear();
    render_draw_snake_full(&s);
    // wall time = 0
    timer_reset();
    // full hunger + calm border
    hunger_init();
    render_draw_time(0);

    food_init(&food, &s);
    // paused=0, bias=0
    pause_reset();

    // Movement pacing
    {
        unsigned char move_interval     = 8u;
        unsigned char frames_since_move = 0u;
        unsigned char sec_since_speedup = 0u;

        while (1) {
            frame_sync_and_input(&dir);

            // --- Pause / Unpause: P pauses, SPACE resumes ---
            if (!pause_is_paused()) {
                if (input_pause_press()) {
                    pause_begin();
                    // calm border while paused
                    VICII->BORDER_COLOR = 14u;
                    render_show_pause();
                }
            } else {
                if (input_unpause_press()) {
                    pause_end_and_account();
                    render_hide_pause();

                    // Instant visual refresh on resume
                    render_draw_snake_full(&s);
                    render_draw_food(food.x, food.y);
                    render_draw_time(game_seconds());
                    hunger_apply_border_now();

                    // Clear any pending second-edge so we don't insta-tick
                    pause_drain_second_edge();
                }
            }

            // --- Movement & collisions (disabled while paused) ---
            if (!pause_is_paused()) {
                frames_since_move++;
                if (frames_since_move >= move_interval) {
                    unsigned char nx, ny;
                    frames_since_move = 0u;

                    // Compute next head cell with wrap-around
                    snake_compute_next_head_wrap(&s, dir, &nx, &ny);

                    // Collision check
                    if (snake_will_self_collide_next(&s, nx, ny)) {
                        // Self-collision: immediate game over
                        // Set border to red
                        VICII->BORDER_COLOR = COL_RED;
                        // wait for 1.5s on PAL (50 Hz) before showing game over
                        wait_frames_blocking(75u);
                        // Game over screen with final time
                        render_game_over(game_seconds());
                        return;
                    }

                    // Eat check
                    if ((nx == food.x) && (ny == food.y)) {
                        // GROW on eat
                        food_handle_eat_grow(&s, dir, &food);
                    } else {
                        // Normal step (no growth)
                        unsigned char old_tail_x, old_tail_y;
                        snake_step(&s, dir, &old_tail_x, &old_tail_y);

                        // We already computed nx,ny before the step; that's the new head cell
                        render_apply_step(old_tail_x, old_tail_y, nx, ny);
                    }
                }
            }

            /* Tick HUD + hunger once per second; end if starved.
               This is pause-aware inside hud_tick(): it drains the edge and returns 0 while paused. */
            if (hud_tick(&move_interval, &sec_since_speedup)) {
                // wait for 1.5s on PAL (50 Hz) before showing game over
                wait_frames_blocking(75u);
                // Game over screen with final time
                render_game_over(game_seconds());
                return;
            }
        }
    }
}

/* --------------------------------------
   MAIN: restart loop
-------------------------------------- */
void main(void) {

    // Show start screen and wait for SPACE
    show_start_and_wait();

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

static void show_start_and_wait(void) {
    render_show_start_screen();

    // Wait for SPACE (frame-synced)
    while (1) {
        keyboard_event_scan();
        if (keyboard_key_pressed(KEY_SPACE)) break;
        wait_frame();
    }

    // Clean screen before starting
    render_clear();
}