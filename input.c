// input.c
// Keyboard input handler for Snake (WASD control on C64)

#include <c64.h>
#include <c64-keyboard.h>
#include "input.h"

// Store the last accepted direction to prevent 180° reversal
static Direction last_dir;

// Initialize the keyboard system and input state
void input_init(void) {
    // Initialize C64 keyboard driver
    keyboard_init();

    // Default direction (snake starts moving right)
    last_dir = DIR_RIGHT;

    // Light blue border for visual confirmation
    VICII->BORDER_COLOR = 14;
}

// Update player input once per frame
// Reads keyboard state, updates direction, prevents instant reversal
void input_update(Direction* dir) {
    // Scan keyboard events for new keypresses
    keyboard_event_scan();

    // Start with the currently active direction
    Direction want = *dir;

    // Handle WASD steering controls
    if (keyboard_key_pressed(KEY_W))      want = DIR_UP;
    else if (keyboard_key_pressed(KEY_S)) want = DIR_DOWN;
    else if (keyboard_key_pressed(KEY_A)) want = DIR_LEFT;
    else if (keyboard_key_pressed(KEY_D)) want = DIR_RIGHT;

    // Prevent reversing direction directly (180° turn)
    if (!((last_dir == DIR_UP    && want == DIR_DOWN) ||
          (last_dir == DIR_DOWN  && want == DIR_UP)   ||
          (last_dir == DIR_LEFT  && want == DIR_RIGHT)||
          (last_dir == DIR_RIGHT && want == DIR_LEFT))) {

        // Accept new direction and update history
        *dir = want;
        last_dir = want;
    }
}
