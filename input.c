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

// Reads the current hardware state of the SPACE key directly from the
// Returns 1 if SPACE is physically held down, else 0.
static unsigned char raw_space_down(void) {
    volatile unsigned char* const CIA1_PRA = (unsigned char*)0xDC00;
    volatile unsigned char* const CIA1_PRB = (unsigned char*)0xDC01;

    unsigned char save = *CIA1_PRA;
    unsigned char down = 0u;

    // 'SPACE': Row 7, Col 4
    *CIA1_PRA = (unsigned char)(255u ^ (1u << 7));
    // col 4 is also used by 'D', but that's fine for our purposes
    if ( ((*CIA1_PRB) & (1u << 4)) == 0u ) down = 1u;

    *CIA1_PRA = save;
    return down;
}

// Reads the current hardware state of the 'P' key directly from the
// Returns 1 if the 'P' key is physically held down, else 0.
static unsigned char raw_p_down(void) {
    volatile unsigned char* const CIA1_PRA = (unsigned char*)0xDC00;
    volatile unsigned char* const CIA1_PRB = (unsigned char*)0xDC01;

    unsigned char save = *CIA1_PRA;
    unsigned char down = 0u;

    // 'P': Row 1, Col 2
    *CIA1_PRA = (unsigned char)(255u ^ (1u << 1));
    // col 2 is also used by 'S', but that's fine for our purposes
    if ( ((*CIA1_PRB) & (1u << 2)) == 0u ) down = 1u;

    *CIA1_PRA = save;
    return down;
}

// Detects a *new physical press* of the 'P' key, used to trigger the pause state.
// Returns 1 exactly once per real press of 'P' (edge detection).
// Requires the key to be fully released before it can trigger again.
unsigned char input_pause_press(void) {   // P only
    static unsigned char armed = 1u;

    if (!armed) {
        // Wait for full release
        if (!raw_p_down()) armed = 1u;
        return 0u;
    }

    // Rising-edge from your keyboard driver (press event)
    if (keyboard_key_pressed(KEY_P)) {
        // Disarm until release
        armed = 0u;
        return 1u;
    }
    return 0u;
}

// Detects a *new physical press* of the SPACE key, used to trigger the unpause state.
// Returns 1 exactly once per real press of SPACE (edge detection).
// Requires the key to be fully released before it can trigger again.
unsigned char input_unpause_press(void) { // SPACE only
    static unsigned char armed = 1u;

    if (!armed) {
        // Wait for full release
        if (!raw_space_down()) armed = 1u;
        return 0u;
    }

    if (keyboard_key_pressed(KEY_SPACE)) {
        // Disarm until release
        armed = 0u;
        return 1u;
    }
    return 0u;
}



