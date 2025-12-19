# TennisScore - Amazfit Bip Tennis Scoring App

A tennis scoring application for the Amazfit Bip smartwatch, built on BipOS. Tracks match scores with support for games, sets, and tie-breaks on the watch's compact display.

## Features

- **Score Tracking**: Points (0, 15, 30, 40, AD), games per set, and multiple sets
- **Deuce & Advantage**: Full support for deuce mode with advantage tracking
- **Tie-Breaks**: Automatic tie-break at 6-6 (first to 7 points)
- **Undo Support**: Swipe left to undo last point (circular buffer supports up to 8 undos)
- **Match History**: Displays completed set results (e.g., "6-4 7-5")
- **Time Tracking**: Shows elapsed time since last point and current time
- **Padel Star Point**: Informational advantage counter display

## Layout

```
┌─────────────────────┐
│   Tennis Score      │  Header
├─────────────────────┤
│      6-4 3-2        │  Previous sets / Current games
├─────────────────────┤
│   40  1-AD  40      │  Points + Advantage counter
├─────────────────────┤
│     Timer: 45       │  Elapsed time since last point
│      14:32:17       │  Current time (cyan)
└─────────────────────┘
```

### Touch Controls

- **Tap left (45%)**: Score point for player 1
- **Tap right (55%)**: Score point for player 2
- **Swipe left**: Undo last point
- **Side button**: Exit app

## Padel Star Point

Displays an informational advantage counter between player scores during deuce/advantage situations. Shows "1-AD", "2-AD", etc. in light gray, with "STAR" in cyan when counter reaches 2. The counter is informational only—users decide whether to apply star point rules.

## Building

### Requirements

- **GNU ARM Embedded Toolchain 9 (2019-q4-major)**: `arm-none-eabi-gcc`
  - Download: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
  - Specific version: `gcc-arm-none-eabi-9-2019-q4-major`
- libbip.a library (placed in `../libbip/`)
- Windows (build.bat) or Linux (Makefile)

### Build Steps

#### Windows

First, install the ARM toolchain:
1. Download from: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
2. Select version: **gcc-arm-none-eabi-9-2019-q4-major** (Windows x86_64)
3. Extract and add to PATH

Then build:

```bash
./build.bat
```

#### Linux

First, install the ARM toolchain:

```bash
cd /tmp && wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 && tar xjf gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 -C ~ && echo 'export PATH=$PATH:~/gcc-arm-none-eabi-9-2019-q4-major/bin' >> ~/.bashrc && source ~/.bashrc
```

Then build:

```bash
make clean
make
```

### Output Files

Both build methods produce:
- `TennisScore.elf` - Executable for Amazfit Bip
- `TennisScore.map` - Symbol map for debugging
- Object files (`.o`) - Intermediate compilation artifacts

## Project Structure

```
TennisScore/
├── main.c                          Main application code
├── main.h                          Data structures and constants
├── build.bat                       Build script for Windows
├── Makefile                        Build script for Linux/Fedora WSL
└── README.md                       This file
```

## Code Architecture

### Core Components

**Data Structures**:
- `struct score_status` - Current game state (points, games, serving player, advantage counter)
- `struct score_history_status` - Circular buffer of up to 8 previous scores for undo
- `struct app_data_` - Main app context with score, time, and UI state

**Key Functions**:
- `show_screen()` - Initialize app and register with BipOS
- `draw_screen()` - Render full UI (header, games, points, advantage counter, clock)
- `dispatch_screen()` - Handle tap and swipe gestures
- `draw_advantage_counter()` - Render star point and advantage counter
- `clear_score()` - Reset for new game
- `set_last_score()` - Undo mechanism

### Game Logic

The app tracks deuce/advantage situations and displays an advantage counter that increments each time a player enters advantage from 40-40. The counter persists through undo operations and resets when a new game starts.

### Memory Management

Uses FreeRTOS dynamic allocation:
- `pvPortMalloc()` for heap allocation
- `vPortFree()` for deallocation
- All strings use BipOS custom functions: `_sprintf()`, `_strlen()`, `_strcpy()`

### Display System

**Fonts**:
- `FONT_LETTER_BIG_6` - Large font for main points display
- `FONT_LETTER_MIDDLE_5` - Smaller font for secondary info (games, advantage counter)

**Colors**:
- `COLOR_YELLOW` - Header and serving indicator
- `COLOR_WHITE` - Games display
- `COLOR_GREEN` - Timer
- `COLOR_AQUA` (cyan) - Clock and star point indicator
- `COLOR_ADVANTAGE_COUNTER` (light gray) - Numeric counters (1-AD, 2-AD, etc.)

**Rendering**:
- Y coordinate is baseline for text top-left
- Centered text uses `text_out_center(text, x_center, y)`
- Custom positioning uses `text_out_font(font, text, x, y, color)`

## References

- **BipOS SDK**: [MNVolkov/libbip](https://github.com/MNVolkov/libbip) - Unofficial Amazfit Bip SDK
- **Padel FIP Star Point**: [Official FIP Announcement](https://www.padelfip.com/2025/12/between-innovation-and-tradition-introducing-the-star-point-the-scoring-system-that-appeals-to-everyone/)
- **Amazfit Bip Hardware**: ARM Cortex-M4 @ ~16MHz, 128KB RAM, small OLED display (typically 80x160 or similar)

## License

Licensed under the MIT License

Copyright (c) 2022-2025

Built on the BipOS application template by [Maxim Volkov](https://github.com/MNVolkov) (2019)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
