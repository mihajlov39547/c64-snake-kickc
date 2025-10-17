# KickC 0.8.6 Playbook — Generic Coding Assistant Guidelines

This playbook distills verified KickC project practices, compiler rules, and fragment-safe conventions.  
It ensures reproducible, valid C code for **KickC 0.8.6+** across **C64 / 6502-family targets**.

---

## Core Principles

- **Write simple, fragment-safe C.** Split complex logic into primitive steps.  
- **Avoid undefined or legacy KickC syntax.** Stick to pure, standard C.  
- **Optimize for clarity and deterministic behavior**, not micro performance.  
- **Test small edits frequently.** Fragment errors appear earlier and isolate better.  

---

## Syntax & Language Rules

- Use standard C forms only. KickC 0.8.6 enforces strict parsing.  
- Function pointer syntax: `void (*fn)(void);`
- Pointer qualifiers order: `char * const p;` (never `const char * p` pretending same semantics).
- Avoid non-standard attributes (`__noinline`, `__interrupt`) unless verified in your toolchain.
- Avoid `(void)x;` for no-ops — just write `x;`.
- Do not use `<`/`>` byte operators — use macros: `BYTE0(x)`, `BYTE1(x)`, `WORD0(x)`, `WORD1(x)`.
- Avoid recursion, dynamic memory, or variable-length arrays; stack space is limited.
- Always include proper headers with guards; forward-declare where possible.

---

## Inline Assembly & Interrupts

- Inline assembly only **inside functions**.  
  ```c
  void ack_irq(void) {
      asm { lda #$01; sta $d019; }
  }
  ```
- Use `.segment Code` only within function bodies if needed.  
- Avoid any global-scope inline assembly.  
- Interrupt routines must:
  - Save/restore A, X, Y.  
  - Acknowledge `$D019` or relevant flag.  
  - Stay minimal — call helpers for logic outside ISR.  
  - End with `RTI`.  
- Prefer raster polling if interrupt syntax fails to parse.

---

## Hardware & Memory Access

- Always `#include <c64.h>` or the proper target header.
- Treat hardware registers and shared flags as `volatile`.
- Prefer **raw address macros** over struct fields for long-term stability:  
  ```c
  #define VIC_RASTER (*((unsigned char*)0xD012))
  ```
- Access VIC-II, CIA, SID, and COLOR RAM directly when needed; field names may differ by version.
- Avoid heavy writes to `SCREEN` ($0400) or `COLOR_RAM` ($D800) each frame. Update only diffs.

---

## Looping & Control Flow

- Use explicit semicolons for empty loops:  
  ```c
  while (VIC_RASTER != 0xFF) ;
  while (VIC_RASTER == 0xFF) ;
  ```
- Avoid `{}` empty bodies (parser bug in KickC 0.8.6).  
- Split compound conditions (`&&`, `||`) into sequential tests.  
- Use explicit underflow guards:  
  ```c
  if (count != 0) count--;
  ```

---

## Bit Operations

- Avoid bitwise NOT (`~`) on bytes — missing fragments in 0.8.6.  
- Use static lookup tables instead:
  ```c
  const unsigned char BIT[8]  = {1,2,4,8,16,32,64,128};
  const unsigned char NBIT[8] = {254,253,251,247,239,223,191,127};
  mask |= BIT[b];  // set
  mask &= NBIT[b]; // clear
  ```
- Use unsigned arithmetic for masks and indices; never mix signed types.

---

## Data & Types

- Use `uint8_t`, `uint16_t` for predictable width.  
- Match widths in expressions (`unsigned char` vs `unsigned int`).  
- Avoid mixed-width arithmetic — cast explicitly when combining.  
- Replace division or modulo with subtract loops or precomputed tables.  
- Keep large tables `static const` for ROM placement.  
- Avoid floating point or `double`; use integer/fixed-point math.

---

## Timing & Frame Sync

- Use **either** timer polling or ISR timing — never both.  
- Wait for raster safely:  
  ```c
  while (VIC_RASTER != 0xFF) ;
  while (VIC_RASTER == 0xFF) ;
  ```
- Maintain consistent per-frame order:  
  `input → logic → collisions → rendering → HUD/time`  
- For time/HUD updates, use one rising-edge check:  
  ```c
  if (timer_second_edge()) update_hud();
  ```

---

## Input Handling

- Scan input once per frame.  
- Prevent immediate 180° reversals (for directional games).  
- Use `volatile` flags to communicate between ISR and main loop.

---

## Rendering & Text

- Write directly to `SCREEN` and `COLOR_RAM`.  
- Cache offsets: `off = y * MAP_W + x;`  
- Use uppercase PETSCII only unless charset changed.  
- For `mm:ss` or counter displays, use subtract loops instead of division.  
- Update HUD only when `timer_second_edge()` triggers.

---

## RNG & Utility Patterns

- Use the SID RNG:
  ```c
  sid_rnd_init();
  unsigned char r = (unsigned char)sid_rnd();
  ```
- Wrap values by subtraction:
  ```c
  while (v >= limit) v -= limit;
  ```
- Toggle values with XOR:
  ```c
  toggle ^= 1;
  ```

---

## Performance & Memory

- Keep stack small; large arrays should be global or static.  
- Cache repeated address calculations or offsets.  
- Avoid runtime math for constants; precompute tables.  
- Keep per-frame logic deterministic — avoid long waits.  
- Do not block outside raster sync or IRQ intervals.

---

## Portability & Maintenance

- Centralize constants (`MAP_W`, `MAP_H`, etc.) in headers.  
- Keep shared enums and types defined once.  
- Use `__address()` only for fixed tables or ZP allocations — verify placement.  
- Avoid platform-specific tricks or undefined C behaviors.  
- Document assumptions (e.g., “MAP_W = 40 for C64 text”).  
- Test on emulator **and** real hardware to confirm timing.  
- Keep builds reproducible (`kickc.bat -a -t c64 -o game.prg main.c`).  

---

## Fragment Error Recovery

When `Missing ASM fragment` occurs:
1. Split the failing expression.  
2. Replace `~`, division, or chained conditions.  
3. Use precomputed tables instead of runtime math.  
4. Avoid post-decrement on 16-bit counters.  
5. Recompile often to isolate cause.

---

## Recommended Order of Execution per Frame

```
1. wait_frame()
2. timer_tick()
3. input_update()
4. movement/logic
5. collisions
6. render updates
7. HUD / timer_second_edge() updates
```

---

## Checklist for Valid KickC 0.8.6 Code

- [x] Standard C syntax only; no legacy operators or casts.  
- [x] Inline assembly inside functions only.  
- [x] No `~`, `<`, `>`, or `(void)` forms in C.  
- [x] Fragment-safe 8-bit math everywhere.  
- [x] Input scanned once per frame.  
- [x] Timing logic single-sourced.  
- [x] Constants centralized in headers.  
- [x] Compiles cleanly under KickC 0.8.6 (`-a -t c64`).  
- [x] Runs correctly in VICE or real hardware.

---

**Version:** KickC 0.8.6 Generic Playbook v2.0  
**Based on:** Verified project code (sys.c, food.c, hud.c, etc.) + prior Copilot and Playbook discussions.  
**Purpose:** Define clear, fragment-safe patterns for human and AI developers targeting 8-bit KickC systems.
