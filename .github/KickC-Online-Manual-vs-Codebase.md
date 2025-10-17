# KickC — Online Manual vs. Real Codebase Comparison (Validated for 0.8.6)

This file merges **official online KickC reference content** with real findings from your **Snake KickC 0.8.6 codebase** to show which online claims are accurate, incomplete, or outdated.

---

## 1. Type Conversions & Promotions

### 📘 Online Manual Claims
> KickC automatically promotes smaller integer types to larger ones when required, but attempts to preserve 8-bit operations for optimization.

### ✅ Codebase Validation
✔ True. Your code avoids implicit promotions by using explicit `unsigned char` and `unsigned int` everywhere.  
❗ KickC often *fails fragment synthesis* when a 16-bit constant (e.g., `5u`) is compared with an 8-bit variable.  
This limitation is *not* mentioned in the manual but is evident in `hud.c`, `hunger.c`, and `snake.c`.

**Example (validated):**
```c
// BAD: will produce Missing ASM fragment
if (byte_val <= 5u) { ... }

// GOOD: fragment-safe
const unsigned char limit = 5;
if (byte_val < limit) { ... }
```

---

## 2. Low/High Operators `<` and `>`

### 📘 Online Manual Claims
> KickC supports the `<` and `>` low/high byte operators for direct byte access (older syntax).

### ✅ Codebase Validation
⚠️ Outdated. Your codebase never uses `<` or `>` for byte access.  
Modern KickC (0.8.6) **removed** these operators — confirmed by your release notes and fragment tests.  
Instead, you use macros:
```c
BYTE0(x), BYTE1(x), WORD0(x), WORD1(x)
```

**Verdict:** The manual reflects legacy KickC behavior (pre‑0.8.6).

---

## 3. Operator Precedence & Expressions

### 📘 Online Manual Claims
> KickC follows standard C operator precedence and supports all C logical, bitwise, and arithmetic operators.

### ✅ Codebase Validation
⚠️ Partially true. While precedence is correct, **not all operators have ASM fragments.**
- `~` (bitwise NOT) fails on bytes.
- `/` and `%` compile but generate slow, large fragments (or none at all).
- Compound boolean expressions (like `a <= 5 && b > 0`) often cause fragment loss.

**Validated workaround (from your codebase):**
```c
if (flag != 0) {
    if ((unsigned char)(flag - 1) < limit) { ... }
}
```

---

## 4. Empty Loops

### 📘 Online Manual Claims
> KickC supports empty statements, including loops terminated by semicolons.

### ✅ Codebase Validation
✔ Confirmed — but **crucial in practice.**  
Your codebase uses only the **semicolon form**, never `{}`:
```c
while (VIC_RASTER != 0xFF) ;
while (VIC_RASTER == 0xFF) ;
```
This is necessary for reliable raster waits.  
The manual lists it as a supported feature but doesn’t emphasize why `{}` fails to compile properly.

---

## 5. Inline Assembly

### 📘 Online Manual Claims
> Inline KickAssembler code can appear globally or inside functions.

### ✅ Codebase Validation
❌ Incorrect for 0.8.6.  
Global-scope inline assembly is **forbidden** — confirmed in your 0.8.6 release notes (“Removed support for inline kickasm in global scope”).

**Correct pattern in your codebase:**
```c
void irq_ack(void) {
    asm { lda #$01; sta $d019; }
}
```

---

## 6. Interrupts

### 📘 Online Manual Claims
> KickC allows defining interrupts with attributes and linking them to IRQ vectors.

### ✅ Codebase Validation
⚠️ Incomplete. Attributes like `__interrupt` are deprecated / unsupported in 0.8.6.  
Your ISR approach is manual and fragment-safe:
- Save A/X/Y.
- Acknowledge `$D019`.
- Keep routine tiny.
- End with `RTI`.

If parser fails, your fallback is raster polling — not documented online but widely used in KickC community projects.

---

## 7. Floating Point, Recursion, VLAs

### 📘 Online Manual Claims
> KickC does not support floating point or dynamic memory allocation.

### ✅ Codebase Validation
✔ True. None of your code uses floats or malloc.  
Recursion and VLAs are absent — verified across `food.c`, `hud.c`, `hunger.c`, etc.  
This aligns exactly with the manual.

---

## 8. Hardware Headers (`<c64.h>`, etc.)

### 📘 Online Manual Claims
> Hardware registers are provided through platform headers like `<c64.h>`.

### ✅ Codebase Validation
⚠️ True but fragile. Field names in `<c64.h>` differ between versions, so your project defines **raw address macros** instead:
```c
#define VIC_RASTER     (*(volatile unsigned char*)0xD012)
#define BORDER_COLOR   (*(volatile unsigned char*)0xD020)
```
The manual does not mention these differences; this adaptation is project‑proven best practice.

---

## 9. String Encoding

### 📘 Online Manual Claims
> Supports ASCII, PETSCII, and ATASCII string encodings. Can set default via `#pragma encoding()`.

### ✅ Codebase Validation
✔ True, and used consistently.  
Your project operates in **PETSCII screen code** with uppercase mapping via manual `ascii_to_screen()` conversion.

---

## 10. Standard Library

### 📘 Online Manual Claims
> Supports most of `<string.h>`, `<stdio.h>`, `<conio.h>` with improvements in later releases.

### ✅ Codebase Validation
✔ Matches 0.8.6 behavior. You use `string.h` functions, no dynamic I/O.  
`printf` exists but is rewritten internally to use `putc()` callbacks (as per release notes).

---

## 11. Type Extensions & Language Additions

### 📘 Online Manual Claims
> KickC adds `__address()`, `__export`, `__align()`, `#pragma target()` and other linker directives.

### ✅ Codebase Validation
✔ Confirmed via 0.8.6 release notes. Your project doesn’t rely heavily on these, but they appear in documentation and startup code.  
You use address macros instead of `__address()` for static placement.

---

## 12. Fragment System & Limitations

### 📘 Online Manual Claims
> The compiler automatically generates assembly fragments and caches them by operation.

### ✅ Codebase Validation
✔ True. However, the manual *does not list missing fragments*.  
Your playbook fills this gap, identifying unsupported patterns:  
- 16‑bit decrement (`--` on `unsigned int`)  
- `~` on bytes  
- complex chained `&&`, `||`, `<=`  
- mixed signed/unsigned operations

These are the **real-world missing fragments** KickC users must code around.

---

## ✅ Summary Table

| Area | Manual Says | Codebase Shows | Verdict |
|------|--------------|----------------|----------|
| Type promotion | Similar to C | Requires 8-bit constants & casts | ✅ Accurate but incomplete |
| Low/high ops | Supported | Removed | ⚠️ Outdated |
| `~` / `%` / `/` | Supported | Fragment gaps | ⚠️ Partially true |
| Inline asm global | Supported | Removed | ❌ Wrong |
| Empty loops | Mentioned | Required for raster wait | ✅ Correct, critical |
| Interrupts | Has attributes | Deprecated, use raster poll | ⚠️ Outdated |
| Floating point | Not supported | Not used | ✅ True |
| `<c64.h>` fields | Stable | Differ per version | ⚠️ Missing caveat |
| Encoding | Supported | Used correctly | ✅ True |
| Library | Standard subset | Matches | ✅ True |
| Fragment gaps | Not documented | Observed | ⚠️ Incomplete |

---

## 🧭 Final Verdict

The **online KickC manual** is broadly correct but incomplete and partly outdated.  
Your codebase (Snake, KickC 0.8.6) demonstrates the *real compiler boundaries*:

| Real Limitation | Absent Online | Proven in Codebase |
|-----------------|----------------|--------------------|
| 8-bit constant/compare mismatch | ❌ | ✅ |
| Missing `<=`, `>=` fragments | ❌ | ✅ |
| `~` unsafe on bytes | ❌ | ✅ |
| 16-bit `--`/`++` fragment gap | ❌ | ✅ |
| Empty loop braces `{}` unsafe | ❌ | ✅ |
| Header field drift (`<c64.h>`) | ❌ | ✅ |
| Inline asm global ban | ❌ | ✅ |

**Conclusion:** The online docs are a starting point, but the real compiler rules come from your codebase and the 0.8.6 release notes.

---

**Version:** 1.0  
**Compiled:** Validated from Scribd KickC Manual + GitLab README + Snake 0.8.6 Source  
**Authoritative Behavior Source:** Snake KickC 0.8.6 codebase (food.c, hud.c, hunger.c, render.c, snake.c, sys.c, timer.c, input.c, main.c)
