#ifndef INPUT_H
#define INPUT_H

// input.h
// Keyboard input interface for Snake (WASD control + reversal lock)

#include "snake.h"   // re-use Direction enum from snake.h

// Initialize keyboard input system
// - Sets up the C64 keyboard driver
// - Initializes direction state and visual border color
void input_init(void);

// Poll keyboard and update movement direction
// - Reads WASD keys for direction control
// - Prevents 180° instant reversal (up/down, left/right)
void input_update(Direction* dir);

// Returns 1 exactly once per *physical* press of 'P' (edge detection).
// Requires the key to be fully released before it can trigger again.
unsigned char input_pause_press(void);

// Returns 1 exactly once per *physical* press of SPACE (edge detection).
// Requires the key to be fully released before it can trigger again.
unsigned char input_unpause_press(void);


#endif
