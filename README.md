# 🐍 C64 Snake (KickC 0.8.6)

A fully playable **Snake game for the Commodore 64**, written in **C using the KickC compiler (version 0.8.6)**.  
This repository serves as a clean, well-commented example on how to structure a small C64 game in modern C with a low-level 8‑bit toolchain.

> **Goal:** Survive as long as possible. The game tracks elapsed time in the HUD (top-left).

---

## ✨ Features

- Text‑mode rendering (40×25) with fast addressing helpers
- SID‑based RNG for food spawning (fragment-safe, no `/` or `%`)
- Smooth keyboard controls (W/A/S/D) with 180°‑reversal lock
- PAL timing (50 Hz) via a simple frame timer
- Modular code layout with clear, function‑level comments

---

## 📦 Project Structure

```
src/
  main.c
  input.c
  input.h
  render.c
  render.h
  snake.c
  snake.h
  food.c
  food.h
  timer.c
  timer.h
```

> All comments are placed on their own lines above the code they describe, making this a good reference for KickC-friendly C coding style.

---

## 🛠 Requirements

- **KickC 0.8.6** (tested with this version)
- A C64 emulator such as **VICE** (e.g., `x64sc.exe`), or real C64 hardware

---

## ⚙️ Build & Run

From the `src/` directory on Windows:

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

- **W / A / S / D** — Up / Left / Down / Right
- **SPACE** or **R** — Restart after Game Over

Reversal safety: Direct 180° turns are blocked (e.g., UP → DOWN on the next frame).

---

## 📚 Educational Focus

- Organizing a multi-file KickC project
- Using VIC‑II and COLOR RAM from C
- Fragment‑friendly arithmetic patterns (e.g., subtract loops instead of division/modulo)
- Clean, readable code that still maps well to 6502

---

## 🧭 Roadmap / TODO

Planned improvements (pull requests welcome):
- **Start screen** — show title/instructions and require **SPACE** to start.
- **Pre–Game Over freeze** — brief 0.5–1.0s pause on collision so the player sees what happened.
- **Pause mechanic** — toggle pause/resume during play (e.g., **P**).
- **Hunger mechanic** — the snake must eat every **12 seconds** or suffer a penalty (Game Over or length decay).
- **Periodic growth** — the snake **grows automatically every 30 seconds**.
- **No food-based growth** — eating does **not** increase length; it only resets hunger (and may affect score).
- **Longer ramp to max speed** — increase the time before top speed (currently ~30s); make the curve more gradual.
- **Prevent food over HUD timer** — exclude the timer’s `(x, y)` cells from spawn positions.
- **Buffs and obstacles** — pickups that **slow** the snake or map tiles that add **extra collision**/hazards.
- **NTSC compatibility** — optional timing adjustment for 60 Hz.
- **Sound/FX** — basic SID beeps for eat / game over / speed-up.
- **Scoring** — augment the survival timer with a score system.

---

## 🧪 Development Tips

- This project uses **KickC 0.8.6**; some comments mention fragment gaps and division/modulo avoidance that are relevant to this version.
- PAL is assumed (`FPS=50`); adjust timers or use a raster IRQ for stable pacing if you later migrate to NTSC.
- `render.h` defines the display constants (`MAP_W=40`, `MAP_H=25`), glyphs, and colors used across modules.

---

## 🪪 License

Released under the **MIT License** — free to use, modify, and redistribute.  
See `LICENSE` for full text (recommended for your public repo).

---

## 👤 Author

**@mihajlov39547** — If you find this repo helpful, please ⭐ it and consider contributing improvements!
