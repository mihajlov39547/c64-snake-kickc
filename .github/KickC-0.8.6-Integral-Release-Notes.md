# KickC **0.8.6** – Integral Release Notes (0.8.3 → 0.8.6)

This document consolidates the official items you provided across **0.8.3**, **0.8.4**, **0.8.5**, and **0.8.6** into a single Markdown changelog.
It preserves wording for accuracy and adds minimal structure (headings, lists).

---

## Contents
- [0.8.6 (Latest)](#086-latest)
  - [Standard C Compatibility](#standard-c-compatibility)
  - [CPUs and Systems](#cpus-and-systems)
  - [Command-line](#command-line)
  - [Standard Library & Examples](#standard-library--example-programs)
  - [Improvements & Fixes](#improvements--fixes)
  - [Upgrade Notes & Migration Tips](#upgrade-notes--migration-tips)
- [0.8.5](#085)
- [0.8.4](#084)
- [0.8.3](#083)

---

## 0.8.6 (Latest)

> This release improves standard C compatibility, adds support for the HUC6280 CPU and overflow of variables to main memory plus a ton of improvements and fixes.

### Standard C Compatibility
- Updated syntax for pointer to function to match standard C `void (*fn)(void)`. #661 (closed) #121 (closed)
- Removed support for the non-standard pointer to function syntax. #661 (closed)
- Updated syntax for const/volatile pointer to match standard C `char * const ptr`. #661 (closed)
- Implemented support for complex casts and parsing complex type names for `sizeof()`. #121 (closed)
- Added support for labels and `goto`. #687 (closed)
- Added support for silent truncation from int/long to char. #684 (closed)
- Added support for `short` / `long` / `signed` types (without `int`). #681 (closed)
- Added support for unsigned integer suffix `U` (e.g., `1234U`). #682 (closed)
- Added support for `sizeof` without parentheses. #695 (closed)
- Added support for automatically converting function names to function pointers. #702 (closed)
- Added support for calling a pointer to function without `(*)`. #692 (closed)
- Changed keyword `export` into `__export`. #572 (closed)
- **Removed low/high operators `<` and `>`**. #221 (closed)
- Introduced `BYTE0(x)`, `BYTE1(x)`, `BYTE2(x)`, `BYTE3(x)` for accessing bytes inside 16/32-bit values. #221 (closed) #667 (closed)
- Introduced `WORD0(x)`, `WORD1(x)` for accessing words inside 16/32-bit values. #221 (closed) #667 (closed)
- Removed support for initializer lists for initializing word/dword. #668 (closed)
- Introduced `MAKEWORD(h,l)`, `MAKELONG(hw,lw)`, `MAKELONG4(hh,hl, lh, ll)` for constructing 16/32-bit values. #668 (closed)
- Added support for character escape sequence `\0`. #642 (closed)
- Added support for character escape sequence `\\`. #504 (closed)
- Added support for `int main(int argc, char **argv)`. #562 (closed)
- Added support for octal character encoding in strings and literal chars. #642 (closed)
- Added support for preprocessor **stringize** operator `#`. #721 (closed)
- Added support for `||`, `&&` and `!` in preprocessor expressions. #646 (closed)
- Added support for names of preprocessor macros with parameters to be used. #693 (closed)
- Added union syntax and support for very simple unions. (Work remains for parameter passing, etc.) #197

### CPUs and Systems
- Added support for CPU **HUC6280** (PC Engine).  
  See https://gitlab.com/jespergravgaard/kickassembler65ce02
- Updated LDQ z-indexing for CPU **45GS02** used in the MEGA65.
- Added support for **VIC20** targets with +3k and +8k memory expansion. #728 (closed)
- Added ISR for **VIC20** and **Plus/4**.

### Command-line
- Added ``-vfixlongbranch`` to give more info about the fix long branch pass.
- Added command line option **-Onolongbranchfix** to disable long branch fixing (relevant for PCEAS). #744 (closed)
- Added command line option **-struct_model** to specify the struct model. #707 (closed)
- Improved error message when passing unknown platform to **-t**. #732 (closed)
- Fixed the problem with **-o** not being honored always. #644 (closed)

### Standard Library & Example Programs
- Added `strncmp()` and `strcmp()`, `memcmp()`, `memchr()` and `strncpy()` to `<string.h>` #700 (closed) #699 (closed)
- Added support for `snprintf()` and `sprintf()`. #715 (closed) #698 (closed)
- Rewrote `printf()` to use a `putc()` function pointer. #698 (closed)
- Added `NULL` pointer to standard library. #647 (closed)
- Added doxygen standard library documentation https://camelot.gitlab.io/kickc/files.html #672 (closed)
- Added improved VERA-lib for CX16 platform by Sven de Velde.
- Re-arranged example programs into platform folders.
- Added example of how to code a ROM in KickC using a linker file and a segment that is compiled but thrown away by the linker. #621 (closed)

### Improvements & Fixes
- Added support for zeropage overflow to main memory when zeropage is exhausted; prioritizes hot variables for ZP. #712 (closed) #753 (closed)
- Added a lot of missing ASM fragments. (Thanks: Eric Majikeyric, Sven de Velde, Sebastian Palm, @IcePic, @CheekyBug.)
- Fixed multiplication rewriting to shift/add to also support `const*var`. #201 (closed)
- Added support for passing `NULL`-parameters. #706 (closed)
- Added `#pragma resource()` for resource files not referenced in inline KickAsm (e.g., linker file). #664 (closed)
- Updated to KickAss **5.24**.
- Address-of handling for procedures: converts to stack call if they have parameters. #121 (closed)
- Support for indirect calls to advanced expressions via a new fragment type. #708 (closed)
- Reserved ZP tmp addresses (`fc–ff`) on all platforms. #737 (closed)
- Added support for non-relocatable main memory variables. #712 (closed)
- Improved sign extension in fragments.
- Improved error message during preprocessor expansion. #683 (closed)
- Advanced indirect calls; calls are now fragments. Support for `@outside_flow` ASM tag placing ASM at end of current scope. Support for expanding label names in ASM fragments. #708 (closed)
- Now synthesizing `_deref_p..c.` fragments using `v..m`. #674 (closed)
- Proper error message on unknown struct member. #638 (closed)
- Intermediate variables now affected by the `var_model`. #666 (closed)
- Fallback to ZP-addressing if an inline ASM instruction does not support absolute addressing. #673 (closed)
- ASM directives are no longer keywords (`resource`/`uses`/`clobbers`/`bytes`/`cycles`). #696 (closed)
- Added support for structs larger than 256 bytes; `sizeof()` becomes word when >256 bytes. #588 (closed)
- Pointers hard-coded to zeropage (e.g., `char * const _s1 = (char*)0xee;`) now generate ZP-addressing mode ASM. #731 (closed)
- Added `struct_model` to platform `.TGT` file. #716 (closed)
- `#pragma struct_model(classic)` fixes struct-with-array instance problems. #590 (closed) #587 (closed)
- Improved error on unfinished hex escape at end of string. #729 (closed)
- Improved parameter type errors to show expected vs. actual. #703 (closed)
- Implemented OutputFileManager to normalize output directory/basename/extension. #663 (closed)
- Improved clobber handling in inline kickasm.
- Improved error message when using unknown variable. #622
- Fixed macros with empty parameter lists. #688 (closed)
- Fixed: variable used inside ASM optimized to a constant. #618 (closed)
- Fixed: erroneously coalescing structs of same size/type affected by address-of. #632 (closed)
- Fixed missing include in `conio` on cx16. #616 (closed)
- Fixed infinite loop from identical strings. #626 (closed)
- Fixed `typedef enum`. #686 (closed) #586 (closed)
- Fixed constantifying struct initializers. #653 (closed)
- Fixed escaping double quotes in chars. #645 (closed)
- Fixed support for address-of on arrays. #662 (closed)
- Fixed auto-casting parameters. #299 (closed)
- Fixed constant bool return causing ASM compile error. #719 (closed)
- Fixed literal strings initializing `char*` in array or struct. #297 (closed)
- Fixed cast of reference to constant string. #298 (closed)
- Fixed illegal call giving exception. #689 (closed)
- Fixed struct containing `char*` member. #397 (closed)
- Fixed pointer addition and constant consolidation producing error “Type inference case not handled byte* + byte*”. #669 (closed)
- Fixed NPE compiling in local directory. #671 (closed)
- Fixed exception “Block referenced, but not found in program” with complex `if(&&)`. #676 (closed)
- Fixed SymbolTypeInference errors without line numbers.
- Fixed mixing stack calls and phi calls mixing up variable versions. #745 (closed)
- Fixed automatic C-file loading including the same file twice. #697 (closed)

### Upgrade Notes & Migration Tips
- Replace any use of `<` / `>` low/high operators with `BYTE0/1/2/3()` and `WORD0/1()`.
- Update any non-standard function pointer syntax to `void (*fn)(void)`.
- Adjust const/volatile pointer declarations: `char * const p;` / `char * volatile p;`.
- Where you relied on initializer lists for word/dword – switch to `MAKEWORD/MAKELONG/MAKELONG4()`.
- If build scripts depend on `export`, rename to `__export`; `align()` ➜ `__align()` (from 0.8.5).

---

## 0.8.5

> Adds Commander X16 support, improved Atari XEX, many features and fixes. Minor breaking changes (double-underscore directive names).

**Highlights**
- Commander X16 target `#pragma target(cx16)`, plus `<conio.h>` and `<veralib.h>`. #581 #606 (closed)
- Atari XEX support (kickass-plugin-atari-xex). #614 (closed)
- `<mega65-dma.h>` for DMA-based memcpy/memset.
- ROM & hardware interrupts support across platforms. #599 (closed)
- Interrupts that only save/restore clobbered registers. #599 (closed)
- `align()` renamed to `__align()` for standard C compliance. #600 (closed)
- `__address()` for arrays at fixed addresses; constant expressions allowed. #217 #211 #596 (closed)
- `#pragma start_address` replaces `#pragma pc`; platforms honor start address. #574 (closed)
- KickAss **5.17**; `-Xassembler`; `@nooptimize` tag; cruncher plugins (ByteBoozer/Exomizer) + examples. #609 #603 (closed)
- Renamed C64 VIC II constants in `<mos6569.h>`.
- GCC-compatible error output for VS Code. #551 (closed)
- Numerous fragment improvements and bug fixes.

---

## 0.8.4

> Adds Atari XL/XE `conio.h`, improves conio on Commodore, many fixes and new fragments.

**Highlights**
- Atari XL `conio.h` by @mark.j.fisher. #546 (closed)
- Initialize conio/stdio cursor position on C64/VIC20/PLUS4/MEGA65. #521 (closed)
- `__KICKC__` define always set. #523 (closed)
- `-Xassembler` passthrough option. #530 (closed)
- `__address(0xfc)` const pointer at hardcoded zeropage. #563 (closed)
- Vars used in ASM auto-converted to `volatile` (be mindful of `char * const` vs `char * volatile`). #554 (closed)
- Plenty of compiler fixes and fragment additions (see detailed list).

---

## 0.8.3

> Adds MEGA65 and Atari XL/XE platforms, 65CE02/45GS02/65C02 CPUs, ASCII/ATASCII encodings, and many improvements.

**Highlights**
- MEGA65 targets: `#pragma target(mega65)` and `mega65_c64`; chipset headers `<mega65.h>`; examples incl. DMA, raster65. #507 (closed)
- Atari XL/XE target (`-t atarixl`), XEX binaries, examples; chipset headers `<atari-xl.h>`. #499 #501 (closed)
- ATASCII & Atari screen-code encodings; ASCII support; default encoding per platform. #500 #503 #263 (closed)
- Program init via `__start()` calling `__init()` and `main()`. #257 (closed)
- `#pragma constructor_for(...)` to run library init before `main()`. #416 (closed)
- `__address()` on arrays; **removed pc parameter on inline kickasm**. #480 (closed)
- Removed support for **global-scope inline kickasm**; use in functions or array initializers. #479 (closed)
- `#pragma` can be parenthesized or not; unknown pragmas ignored. #512 #324 (closed)
- Added **empty statement** loops with just `;`. #395 (closed) #267 (closed)
- Trailing commas in initializers; ranged for-loop with undeclared loop var; numerous compiler fixes.
- Upgraded to KickAssembler 5.16-65CE02; added fragments for new CPUs.

---

**Document scope:** This file consolidates and formats your provided release text for quick reference in one place, without altering the technical meaning.
