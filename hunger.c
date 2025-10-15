#include <c64.h>
#include "hunger.h"

// Internal hunger state
static unsigned char hunger_remaining    = HUNGER_LIMIT_SEC;
// Used to toggle border color in warning state
static unsigned char hunger_flash_toggle = 0u;

// Initialize hunger state: full hunger + calm border
void hunger_init(void) {
    hunger_remaining    = HUNGER_LIMIT_SEC;
    hunger_flash_toggle = 0u;
    VICII->BORDER_COLOR = COL_LIGHT_BLUE;
}

// Restore hunger to full and set calm border (called on food eat)
void hunger_reset_on_feed(void) {
    hunger_remaining    = HUNGER_LIMIT_SEC;
    hunger_flash_toggle = 0u;
    VICII->BORDER_COLOR = COL_LIGHT_BLUE;
}

// Call exactly once per timer_second_edge()
// Returns 1 on starvation (hunger hits 0), else 0.
unsigned char hunger_tick_and_flash(void) {
    if (hunger_remaining != 0u) {
        hunger_remaining--;
    }

    // Border update in warning window
    if (hunger_remaining != 0u) {
        const unsigned char warn = (unsigned char)HUNGER_WARN_START;
        unsigned char hr = hunger_remaining;

        // Flash if in warning state
        if ((unsigned char)(hr - 1u) < warn) {
            hunger_flash_toggle = (unsigned char)(1u - hunger_flash_toggle);
            VICII->BORDER_COLOR = (hunger_flash_toggle ? COL_PINK : COL_RED);
        } else {
            VICII->BORDER_COLOR = COL_LIGHT_BLUE;
        }
    }

    // Starvation check
    if (hunger_remaining == 0u) {
        VICII->BORDER_COLOR = COL_RED;
        return 1u;
    }

    // Not yet starved
    return 0u;
}

// Immediately set the border to the correct color for the current hunger state
void hunger_apply_border_now(void) {
    const unsigned char warn = (unsigned char)HUNGER_WARN_START;
    unsigned char hr = hunger_remaining;

    if (hr == 0u) {
        VICII->BORDER_COLOR = COL_RED;
        return;
    }

    // Flash if in warning state
    if ((unsigned char)(hr - 1u) < warn) {
        VICII->BORDER_COLOR = (hunger_flash_toggle ? COL_PINK : COL_RED);
    } else {
        VICII->BORDER_COLOR = COL_LIGHT_BLUE;
    }
}
