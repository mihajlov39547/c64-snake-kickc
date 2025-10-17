#ifndef FOOD_H
#define FOOD_H

#include <stdint.h>
#include "snake.h"
#include "render.h"

// A single food item on the grid
typedef struct {
    uint8_t x;
    uint8_t y;
} Food;

// Initialize food system:
// - Stir the SID RNG so first values differ across runs
// - Spawn the first food on a free cell
// - Draw the food
void food_init(Food* f, const Snake* s);

// Respawn food at a new random free cell
// Note: does not draw; caller may draw after moving/animating
void food_spawn(Food* f, const Snake* s);

// Handle eating food WITH growth:
// - Grow step (tail not removed)
// - Hunger reset and calm border
// - Respawn and draw new food
void food_handle_eat_grow(Snake* s, Direction dir, Food* food);

// Return a random 8-bit value using SID voice 3
// Initializes the SID RNG on first call
uint8_t rng8(void);

#endif
