# KickC vs. Standard C — A Practical, Codebase‑Driven Guide (for 0.8.6)

> Purpose: teach a C programmer *what changes* when targeting 6502 systems with **KickC 0.8.6**, using your Snake codebase as the ground truth.  
> Scope: **language & compiler behavior**, **codegen fragments**, **hardware access patterns**, and **idiomatic rewrites**.

---

## 0) TL;DR: The Big Differences

| Area | Standard C (e.g., GCC/Clang) | KickC 0.8.6 (observed in codebase) |
|---|---|---|
| Integer promotions | Liberal; 8-bit often promoted to `int` | **Stay 8-bit intentionally**; avoid mixed-width compares; explicit casts |
| Operators | `<=`, `>=`, `~`, `/`, `%` usually fine | **Fragment gaps**: prefer subtraction-compare; avoid `~` on bytes; avoid `/`/`%` in hot code |
| Low/high byte ops | Can do shifts/casts freely | **`<`/`>` removed**; use `BYTE0/1` / `WORD0/1` macros |
| Empty loops | `{}` or `;` both OK | **Use semicolon form** `while (expr) ;` (parser/fragment safety) |
| Function pointers | Multiple syntaxes accepted | **Standard syntax only** `void (*fn)(void);` |
| `(void)x;` no‑op | Common idiom | **Not supported**; just write `x;` |
| Inline asm | Sometimes allowed at global scope | **Only inside functions**; segments inside function if needed |
| Interrupts | Attributes/pragmas common | **Keep minimal; ack source; save AXY; RTI; or raster‑poll** |
| Floating point | Available | **Avoid** (no FPU; heavy; not idiomatic) |
| Recursion/VLAs | Allowed | **Avoid** (tiny stacks; limited RAM) |
| Headers & HW | Stable structs in SDKs | **Header fields vary**; prefer **raw addresses** for VIC/CIA/SID |
| Libraries | `printf` everywhere | Use *light* routines or `printf` via putc hook; prefer tables |
| Encoding | ASCII/UTF-8 | **C64 screen codes / PETSCII**; uppercase; convert text manually |

---

## 1) Types & Integer Promotions

### What Standard C Does
- Freely promotes small integers to `int`; `<=`/`>=`/mixed types typically produce correct code.

### What KickC 0.8.6 Expects
- **Stay in 8-bit** where possible for indexes/counters.  
- Mixed-width compares (e.g., `uint8_t x <= 5u`) can trigger **Missing ASM fragment** because `5u` is 16-bit.
- Use 8-bit constants / 8-bit variables consistently.

**Codebase examples**
```c
// GOOD (hud.c): 8-bit compares & increments
(*sec_since_speedup)++;
if ((unsigned char)(*sec_since_speedup) >= (unsigned char)10) {
    if ((unsigned char)(*move_interval) > (unsigned char)2) {
        (*move_interval)--;
    }
}
```

**Canonical rewrite patterns**
```c
// Standard C (fine there, risky here)
if (byte_val <= 5u) { /* ... */ }

// KickC-safe
const unsigned char limit = 5;
if (byte_val < limit) { /* ... */ }
```

---

## 2) Operators & Expressions

### Standard C
- `<=`, `>=`, `~`, `%`, `/` work; complex boolean chains are OK.

### KickC Rule of Thumb
- `<=` / `>=` → **rewrite using subtraction + `<`** with underflow guard:
```c
if (v != 0 && (unsigned char)(v - 1) < limit) { /* v <= limit */ }
```
- Avoid `~` on bytes → use **BIT / NBIT tables**.
- Avoid `%` and `/` in hot loops → use **subtract loops** or tables.
- **Split compound conditions** (`&&`, `||`) into sequential tests.

**Codebase examples**
```c
// hunger.c — warning window without <=
if ((unsigned char)(hr - 1u) < warn) { /* ... */ }

// render.c — mm:ss without division/modulo
while (secs >= 60u) { secs -= 60u; if (mm < 99) mm++; else break; }
```

---

## 3) Low/High Byte Handling

### Standard C
- Often uses shifts or ad‑hoc casts to peel bytes.

### KickC 0.8.6
- **`<`/`>` operators removed**. Use macros **BYTE0/1/2/3**, **WORD0/1** from KickC.
- Prefer simple casts only when necessary; keep expressions primitive.

---

## 4) No‑Op Idioms

### Standard C
- `(void)x;` to silence “unused” warnings.

### KickC 0.8.6
- `(void)` casts not supported as no‑ops → **Just write the expression**:
```c
void foo(unsigned char p) { p; }  // touches p
```

---

## 5) Inline Assembly & Interrupts

### Standard C Ecosystem
- Pragmas/attributes like `__interrupt`, `naked`, global asm sections often used.

### KickC 0.8.6
- **Inline asm only in functions**; if segments required, keep inside the function.  
- ISRs: **save A/X/Y**, **ack** (e.g., `$D019`), **tiny work only**, `RTI`.  
- If syntax/vector setup gives trouble, **use raster polling**.

**Codebase patterns**
```c
// sys.c — raster wait using semicolon empty statements
while (VIC_RASTER != 0xFF) ;
while (VIC_RASTER == 0xFF) ;
```

---

## 6) Empty Loops (Parsing Quirk)

- Use **semicolon statement** for empty loop bodies:
```c
while (RASTER != 0xFF) ;
while (RASTER == 0xFF) ;
```
- Avoid `{}` empty bodies (less reliable in 0.8.6).

---

## 7) Hardware Access & Headers

### Standard C
- Stable struct fields in vendor SDK; often memory‑mapped structs.

### KickC 0.8.6 Reality
- `<c64.h>` field names can **change across versions**. Codebase uses both forms:
  - Struct style: `VICII->BORDER_COLOR = 14;` (convenient, **but brittle**)
  - **Raw addresses (preferred)** for durability:
    ```c
    #define RASTER       (*(volatile unsigned char*)0xD012)
    #define BORDER_COLOR (*(volatile unsigned char*)0xD020)
    BORDER_COLOR = 14u;
    ```
- Mark HW registers / shared flags **`volatile`**.

---

## 8) Strings, Text, and Encoding

### Standard C
- ASCII/UTF‑8 terminals; `printf` ubiquitous.

### KickC 0.8.6 on C64
- **C64 screen codes / PETSCII**; uppercase preferred.  
- Convert ASCII manually when needed (see `render.c::ascii_to_screen()`).
- UI text ≤ 40 characters, one line.

**Codebase example**
```c
static unsigned char ascii_to_screen(unsigned char ch) {
    unsigned char t = (unsigned char)(ch - (unsigned char)65);
    if (t <= (unsigned char)25) return (unsigned char)(t + (unsigned char)1);
    if (ch == (unsigned char)32) return (unsigned char)32;
    return ch;
}
```

---

## 9) Libraries & I/O

- `printf` exists but was rewritten to use a `putc()` function pointer in 0.8.6.  
- For performance‑critical paths, prefer **direct screen writes** and **tiny helpers** (`pset`, `pchar`).

**Codebase** uses precomputed row offsets to avoid multiplies:
```c
static inline unsigned int paddr(unsigned char x, unsigned char y) {
    return row_off[y] + x;
}
```

---

## 10) RNG: Don’t Roll Your Own LFSR

- Standard C: `rand()` typical.
- KickC best practice: use **SID RNG** (`sid_rnd_init()`, `sid_rnd()`), as in `food.c`:
```c
if(!g_rng_inited) { sid_rnd_init(); g_rng_inited = 1; }
return (uint8_t)sid_rnd();
```

---

## 11) Control Flow & Game Loop Contract

- **Single timing authority:** either poll a timer each frame or tick it in IRQ, not both.
- **Once‑per‑second edge:** centralize HUD/time/hunger in one place (`timer_second_edge()`).
- **Per‑frame order:** `input → movement → collisions → render → HUD/time` (codebase: `main.c`, `hud.c`).

---

## 12) Data Structures & Memory

- Avoid recursion, VLAs, and big stack arrays. Use `static` or globals for large tables.
- Keep `uint8_t` / `uint16_t` widths explicit for data mapped to hardware.
- Occupancy grid uses **bit tables** instead of `~` (see `snake.c`: `BIT[]` / `NBIT[]`).

---

## 13) Common Standard‑C → KickC Rewrites (Cookbook)

### A) “Less‑than‑or‑equal” compare (8‑bit)
```c
// Standard C
if (x <= k) { ... }

// KickC‑safe
if (x != 0 && (unsigned char)(x - 1) < k) { ... }
```

### B) Toggle
```c
// Standard C
toggle = (toggle + 1) % 2;

// KickC‑safe
toggle ^= 1;
```

### C) Divide/modulo by 60 (time)
```c
// Standard C
mm = secs / 60; ss = secs % 60;

// KickC‑safe
mm = 0;
while (secs >= 60u) { secs -= 60u; if (mm < 99) mm++; else break; }
ss = (unsigned char)secs;
```

### D) Bit clear without `~`
```c
// Standard C
byte &= ~mask;

// KickC‑safe
byte &= NBIT[idx];
```

### E) Raster wait
```c
while (RASTER != 0xFF) ;
while (RASTER == 0xFF) ;
```

---

## 14) “What You *Can* Still Do” (Often Asked)

- **Labels/goto:** supported in 0.8.6.  
- **`sizeof` without parentheses:** supported.  
- **Unions:** *very simple* unions supported; avoid passing unions as parameters.  
- **Function pointers:** standard syntax; you can call via the pointer without `(*)`.  
- **Preprocessor:** macro stringize `#`, logical `||`, `&&`, `!` in expressions.  
- **`NULL`** exists; `string.h` adds more routines in 0.8.6 (`strncmp`, `memcmp`, etc.).

---

## 15) Golden Rules Checklist

- [ ] Use 8‑bit constants with 8‑bit variables.  
- [ ] Replace `<=`/`>=` with subtract‑then‑compare.  
- [ ] Split compound boolean chains into sequential tests.  
- [ ] Use semicolon empty loops for raster waits.  
- [ ] Avoid `~` on bytes; use `BIT`/`NBIT`.  
- [ ] Avoid `%`/`/` in hot paths; use subtract loops/tables.  
- [ ] Inline asm only inside functions; ISRs are tiny, ack, RTI.  
- [ ] Prefer raw HW addresses; mark as `volatile`.  
- [ ] Convert text to screen codes; stay uppercase.  
- [ ] No `(void)x;` — just `x;`.  
- [ ] No recursion / VLAs; keep stack tiny.  
- [ ] Centralize once‑per‑second logic on `timer_second_edge()`.  

---

## 16) Minimal KickC‑Safe Template

```c
#include <c64.h>
#include <stdint.h>

#define RASTER         (*(volatile uint8_t*)0xD012)
#define BORDER_COLOR   (*(volatile uint8_t*)0xD020)

static inline void wait_frame(void) {
    while (RASTER != 0xFF) ;
    while (RASTER == 0xFF) ;
}

static uint8_t toggle;

void main(void) {
    BORDER_COLOR = 14u;     // light blue
    for (;;) {
        wait_frame();
        toggle ^= 1;
        BORDER_COLOR = toggle ? 10u : 2u; // PINK/RED
    }
}
```

---

**Version:** KickC vs Standard C (0.8.6) — Comprehensive Comparison v1.0  
**Ground truth:** your Snake codebase (`main.c`, `render.c`, `hunger.c`, `hud.c`, `food.c`, `snake.c`, `sys.c`, `timer.c`, `input.c`) and verified 0.8.6 rules.
