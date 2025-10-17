# KickC 0.8.6 – Lessons Learned: Hunger Timer & Compiler-Safe Patterns

This document consolidates the final, verified compiler-safe rules for KickC 0.8.6 — covering fragment-safe comparison logic, 8-bit constants, and runtime behavior patterns (based on real Hunger Timer fixes).

---

## ✅ DO

### Type Safety & Comparisons
- Always use **8-bit constants with 8-bit variables** in comparisons.  
  Example:  
  ```c
  const unsigned char warn = 5;
  if (hunger_remaining < warn) { ... }
  ```

- Replace `<=` with a **fragment-safe subtraction comparison**.  
  Example:  
  ```c
  if (hunger_remaining != 0 &&
      (unsigned char)(hunger_remaining - 1) < warn) {
      /* safe equivalent to hunger_remaining <= warn */
  }
  ```

- **Split compound conditions** into simpler sequential tests to match known compiler fragments.  
  Example:  
  ```c
  if (hunger_remaining != 0) {
      if ((unsigned char)(hunger_remaining - 1) < warn) { ... }
  }
  ```

### Timer Logic
- Keep all **once-per-second logic** behind a single call to `timer_second_edge()`.  
  Example:  
  ```c
  if (timer_second_edge()) {
      update_time();
      update_hunger();
      update_border_flash();
  }
  ```

### Visual & State Updates
- Use **simple XOR toggles** instead of division or modulo for blinking or alternating visuals.  
  Example:  
  ```c
  flash ^= 1;
  BORDER_COLOR = flash ? COL_PINK : COL_RED;
  ```

- **Guard underflow** when decrementing unsigned counters.  
  Example:  
  ```c
  if (counter != 0) counter--;
  ```

- Restore **normal border color** after a warning period.  
  Example:  
  ```c
  BORDER_COLOR = COL_LIGHT_BLUE;
  ```

- Reset **hunger state** on food eaten (counter, toggle, border).  
  Example:  
  ```c
  hunger_remaining    = (unsigned char)12;
  hunger_flash_toggle = 0;
  BORDER_COLOR        = COL_LIGHT_BLUE;
  ```

### Timing & Raster Sync
- Keep **raster waits** as explicit semicolon statements (not `{}`).  
  Example:  
  ```c
  while (RASTER != 0xFF) ;
  while (RASTER == 0xFF) ;
  ```

---

## ❌ DON’T

- Don’t mix **8-bit variables** with **16-bit literals** (e.g., `5u`, `12u`) in comparisons — it triggers missing ASM fragments.  
  Example (bad):  
  ```c
  if (hunger_remaining <= 5u) { ... }  // ❌ 5u = 16-bit
  ```

- Don’t use `<=` or `>=` in multi-condition expressions. Split comparisons into smaller steps.

- Don’t combine multiple relational operators (`&&`, `||`, `<=`, `>=`) in one line; KickC may fail to fragment them.

- Don’t call `timer_second_edge()` more than once per frame; handle all once-per-second logic in one unified block.

- Don’t use `%` or division for toggling visuals — use XOR (`^=`) or bit masks.

- Don’t rely on implicit type promotions — **cast explicitly** when mixing widths.  
  Example:  
  ```c
  unsigned char r = (unsigned char)(val16 & 0xFF);
  ```

- Don’t compare directly against arithmetic expressions like `(warn + 1)` — precompute them as constants.

- Don’t use braces `{}` for empty loops — always use semicolon form.

---

## ⚙️ Constants & Safe Macros

To avoid header drift across KickC versions, define stable hardware macros at the top of your modules:

```c
#define RASTER         (*(volatile unsigned char*)0xD012)
#define BORDER_COLOR   (*(volatile unsigned char*)0xD020)
#define COL_LIGHT_BLUE 14
#define COL_RED         2
#define COL_PINK       10
```

Then use them consistently:
```c
flash ^= 1;
BORDER_COLOR = flash ? COL_PINK : COL_RED;

if (timer_second_edge()) update_hud();

while (RASTER != 0xFF) ;
while (RASTER == 0xFF) ;
```

---

## 💡 Optional Helper Pattern

If you frequently perform fragment-safe decrement/compare operations, use a helper inline:

```c
static inline unsigned char decr_and_below(unsigned char v, unsigned char lim) {
    return (v != 0) && ((unsigned char)(v - 1) < lim);
}
```
Usage:
```c
if (decr_and_below(hunger_remaining, warn)) {
    // warning window logic
}
```

---

## 📘 Summary Checklist

- [x] Use 8-bit constants for all 8-bit compares.  
- [x] Replace `<=` / `>=` with subtraction-based comparisons.  
- [x] Split chained logical operators into sequential checks.  
- [x] Single `timer_second_edge()` logic block per frame.  
- [x] XOR toggles for visual effects.  
- [x] Guard unsigned underflow.  
- [x] Explicit semicolon loops for raster waits.  
- [x] Use raw register macros (`0xD012`, `0xD020`) for reliability.  

---

**Version:** KickC 0.8.6 — Lessons Learned v1.0  
**Context:** Hunger Timer fragment behavior and timing fixes for KickC 0.8.6  
**Goal:** Ensure consistent, fragment-safe code generation and predictable 1 Hz game logic updates.
