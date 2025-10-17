# Copilot Coding Instructions — KickC Generic Edition (C64 / 6502 Targets)

These instructions define how AI coding assistants should write **KickC 0.8.6-compatible C code** for 8-bit systems like the **Commodore 64**.  
They combine verified project practices, compiler playbooks, and library conventions into one unified guide.

---

## 1. Overview

KickC is a C-like compiler targeting **6502-family CPUs** (C64, C128, VIC20, MEGA65, Atari XL, etc.).  
It enforces stricter C syntax than typical compilers and uses code *fragments* to generate optimized 6502 assembly.  
Your goal is to write **fragment-friendly C**, simple and static enough for the compiler to synthesize ASM.

---

## 2. Build & Run Basics

- **Compile single file:**  
  `kickc.bat -a -t c64 -o program.prg main.c`
- **Compile multi-file project:**  
  `kickc.bat -a -t c64 -o snake.prg main.c render.c snake.c input.c timer.c food.c`
- **Run in emulator:**  
  Use VICE (`x64.exe`) to load and run the generated `.prg` binary.

Output artifacts usually include `.prg`, `.asm`, `.dbg`, `.klog`, and `.vs` files.

---

## 3. Coding Rules for KickC 0.8.6

### ✅ Use Standard C Syntax
- Function pointers: `void (*fn)(void);`
- Pointer qualifiers in normal order: `char * const p;`
- Avoid casts in macros or loops.  
- Avoid `const char *` pretending to mean “pointer-to-const.”

### ✅ Hardware Access
- Include `<c64.h>` or target headers; **field names may differ** between versions.
- Use raw addresses for stability, e.g.  
  `#define VIC_RASTER (*((unsigned char*)0xD012))`
- Always mark hardware registers and shared flags as `volatile`.

### ✅ Inline Assembly
- Only valid **inside functions**.  
  Example:
  ```c
  void irq_ack(void) {
      asm { lda #$01; sta $d019; }
  }
  ```
- Never at global scope.

### ✅ Interrupts
- If vector setup fails, use **raster polling** instead of hardware IRQ.
- If using an IRQ:
  - Save/restore A,X,Y.
  - Acknowledge `$D019` (e.g., write `#$01`).
  - Keep routine tiny and end with `RTI`.

### ✅ Byte and Word Parts
Use macros instead of `<` or `>` operators:
```c
BYTE0(x), BYTE1(x), WORD0(x), WORD1(x)
```

### ✅ Loops
Use semicolons for empty bodies:
```c
while (VIC_RASTER != 0xFF) ;
while (VIC_RASTER == 0xFF) ;
```

### ✅ Bit Operations
Avoid `~` — use tables:
```c
static const unsigned char BIT[8]  = {1,2,4,8,16,32,64,128};
static const unsigned char NBIT[8] = {254,253,251,247,239,223,191,127};
mask |= BIT[b];   // set bit
mask &= NBIT[b];  // clear bit
```

### ✅ Screen & Memory
- Cache offsets like `off = y * MAP_W + x` once per draw.  
- Prefer direct writes to `SCREEN` ($0400) and `COLOR_RAM` ($D800).  
- Update only changed cells per frame.

### ✅ Timing & Game Loop
- Use either timer polling **or** ISR-driven timing — not both.  
- Expose a rising-edge helper:  
  ```c
  if (timer_second_edge()) update_hud();
  ```
- Main loop order: **input → logic → collisions → render → HUD/time**.

### ✅ Input
- Scan once per frame.  
- Prevent 180° reversals (snake-style games).

### ✅ Types & Math
- Favor unsigned types: `uint8_t`, `uint16_t`.
- Avoid mixed-width comparisons or operations.  
  Example: cast both sides explicitly to `unsigned char`.
- Replace division/modulo with subtract loops or lookup tables.

### ✅ Memory & Stack
- Keep locals small.  
- Large arrays = `static` or global.  
- Avoid recursion.

---

## 4. DO Guidelines

- ✅ Use `volatile` for flags changed by ISRs.  
- ✅ Use `const` for read-only tables (ROM-eligible).  
- ✅ Use `unsigned char` for counters, `unsigned int` for frame counts.  
- ✅ Split expressions into small, clear steps.  
- ✅ Define constants once in headers (e.g., `MAP_W`, `MAP_H`).  
- ✅ Handle one logical frame per raster cycle.  
- ✅ Use SID RNG safely:
  ```c
  sid_rnd_init();
  unsigned char r = (unsigned char)sid_rnd();
  ```
- ✅ Implement once-per-second logic via `timer_second_edge()`.  
- ✅ For toggles, use XOR:
  ```c
  state ^= 1;
  ```
- ✅ Guard underflow:
  ```c
  if (count != 0) count--;
  ```

---

## 5. DON’T Guidelines

- ❌ Don’t use `(void)x;` — use `x;` as a no-op.  
- ❌ Don’t use `~` bitwise NOT.  
- ❌ Don’t use recursion or dynamic memory.  
- ❌ Don’t mix signed and unsigned arithmetic.  
- ❌ Don’t rely on `<` or `>` low/high operators.  
- ❌ Don’t chain `&&`/`||` in complex expressions — split them.  
- ❌ Don’t use modulo or division inside loops.  
- ❌ Don’t block outside raster-sync.  
- ❌ Don’t perform heavy rendering each frame — update diffs only.  
- ❌ Don’t assume field names in `<c64.h>` remain stable.  

---

## 6. Fragment Error Fix Strategy

If you see `Missing ASM fragment` errors:

1. Split expressions into smaller statements.  
2. Replace exotic operations with simpler ones.  
3. Use precomputed tables for math or strings.  
4. Avoid 16-bit decrements on unsigned ints (`frames = frames - 1u` instead of `frames--`).  
5. Test after every small change.

---

## 7. Rendering & Text

- Write to `SCREEN` ($0400) and `COLOR_RAM` ($D800).  
- Keep strings uppercase PETSCII.  
- Avoid lowercase unless charset is changed.  
- Use subtract loops for `mm:ss` time display.  
- Keep HUD updates lightweight (only when second edge triggers).

---

## 8. Example Patterns

**Frame Wait:**
```c
#define VIC_RASTER (*((unsigned char*)0xD012))
void wait_frame(void) {
    while (VIC_RASTER != 0xFF) ;
    while (VIC_RASTER == 0xFF) ;
}
```

**Random Spawn:**
```c
uint8_t wrap_under(uint8_t v, uint8_t limit) {
    while (v >= limit) v -= limit;
    return v;
}
```

**Safe Decrement:**
```c
if (counter != 0u) counter--;
```

**No-op Parameter Touch:**
```c
void foo(uint8_t p) { p; }
```

---

## 9. Summary Checklist

- [ ] Code compiles cleanly under KickC 0.8.6 (`-a -t c64`).  
- [ ] No `(void)` casts, `<`/`>` ops, or `~` masks.  
- [ ] Input scanned once per frame.  
- [ ] Rendering minimal and stable.  
- [ ] Timer logic single-sourced.  
- [ ] Constants centralized in headers.  
- [ ] Verified on VICE — timing correct.

---

**Version:** Generic KickC Copilot Rules v1.0  
**Base:** KickC 0.8.6 Playbook + Project Ground Truth  
**Purpose:** Ensure reproducible, fragment-safe KickC code generation.
