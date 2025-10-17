# 🐍 C64 Snake (KickC 0.8.6)

A fully playable **Snake game for the Commodore 64**, written in **C using the KickC compiler (version 0.8.6)**.  
This project demonstrates clean modular architecture, hardware-safe timing, and modern C structure on an 8-bit platform.

> **Goal:** Survive as long as possible by eating food to stay alive — hunger counts down every second!  
> The HUD shows elapsed time and speed ramping. Don’t starve!

---

## ✨ Features

- Text-mode rendering (40×25) with fast address computation  
- SID-based RNG for fragment-safe food placement  
- Smooth keyboard input (W/A/S/D) with 180°-reversal protection  
- Hunger mechanic with flashing border warning and starvation state  
- Pause/resume system (toggle `P` / `SPACE`) with pause-adjusted timer  
- HUD displaying elapsed time and automatic speed-up curve  
- PAL-synced frame pacing via raster polling  
- Clean modular code layout (each system isolated in its own file)

---

## 📦 Project Structure

```
src/
  main.c          – main loop / integration
  sys.c, sys.h    – frame sync + input/timer tick
  input.c, input.h– directional + pause input
  snake.c, snake.h– snake state & movement
  food.c, food.h  – food spawn & eat logic
  render.c, render.h– text-mode drawing
  timer.c, timer.h– frame/second timer
  hud.c, hud.h    – per-second HUD updates (time/speed/hunger)
  hunger.c, hunger.h– hunger countdown + border flash
  pause.c, pause.h– pause state & time bias tracking
```

> Each module is self-contained and documented.  
> All comments precede the code they describe — KickC-friendly style for clarity and ASM readability.

---

## 🛠 Requirements

- **KickC 0.8.6** compiler  
- **VICE** (C64 emulator) or real C64 hardware

---

## ⚙️ Build & Run

From the `src/` directory:

```bat
REM Build (KickC 0.8.6)
kickc.bat *.c -t c64 -a -o snake.prg

REM Run in VICE
x64sc.exe -autostart .\snake.prg
```

### Notes
- `-t c64` targets the C64 memory model.
- `-a` emits the assembly listing alongside the binary (useful for learning).
- Output is `snake.prg`, suitable for emulators and most loaders.

---

## 🎮 Controls

| Key | Action |
|-----|---------|
| **W / A / S / D** | Move Up / Left / Down / Right |
| **P** | Pause game |
| **SPACE** | Resume after pause |
| **R** | Restart after Game Over |

🧠 Reversal safety: cannot instantly reverse direction (e.g. UP→DOWN).  
⚠️ If you don’t eat within 12 seconds, you starve! The border flashes red/pink as a warning.

---

## 📚 Educational Highlights

- Modular multi-file KickC project structure  
- Using **VIC-II raster sync** and **CIA timers** from C  
- Implementing per-second game logic (`timer_second_edge()`)  
- Handling **pause bias** to keep time accurate  
- Safe 8-bit arithmetic (no `/` or `%`, only tables / loops)  
- KickC 0.8.6 compliance (no `<`/`>` low/high operators, no `(void)` casts)  
- Readable, efficient 6502 code

---

## 🧭 Roadmap / TODO

Next improvements planned:
- **Buffs and obstacles** — pickups that **slow** the snake or map tiles that add **extra collision**/hazards.
- **NTSC compatibility** (timing adjust for 60 Hz)  
- **Basic SID FX** (eat / starve / speed-up tones)  
- **High-score screen** with initials entry  

---

## 🧪 Development Tips

- Keep per-frame order stable: **input → logic → render → HUD**.  
- Use `wait_frame()` or `frame_sync_and_input()` for PAL 50 Hz pacing.  
- For code clarity, each system exposes a minimal API (`*.h`) and hides state in `*.c`.  
- Refer to **KickC 0.8.6 Playbook** in this repo for compiler-specific dos and don’ts.

---

## 🪪 License

Released under the **MIT License** — free to use, modify, and distribute.  
See `LICENSE` for full text.

---

## 👤 Author

**@mihajlov39547**  
If you enjoy this project, please ⭐ the repo and share improvements!
