# Rachel DOS Edition - Platform #002

**CGA Graphics in 4-color glory!**

## Overview

The DOS implementation of Rachel, supporting CGA/EGA/VGA graphics and running on everything from DOS 3.3 to FreeDOS. This is Platform #002 in the Rachel Multiverse - proving that 640K is indeed enough for anybody to play Rachel.

## Features

- Pure DOS implementation with CGA graphics support
- ASCII card rendering with DOS box-drawing characters
- AI opponent with strategic play
- Keyboard controls for intuitive gameplay
- Runs on any DOS-compatible system from 1981 onwards

## Building

### Requirements
- DJGPP (DOS port of GCC) or Turbo C
- DOS 3.3 or later (FreeDOS works great)
- CGA/EGA/VGA graphics card

### Compilation

#### Using DJGPP:
```bash
djgpp rachel.c ../rachel-core/rules.c -o rachel.exe
```

#### Using Turbo C:
```
tcc rachel.c ..\rachel-core\rules.c
```

#### Cross-compile from modern system:
```bash
# Install DJGPP cross-compiler
# Then:
i586-pc-msdosdjgpp-gcc rachel.c ../rachel-core/rules.c -o rachel.exe
```

## Running

### On real DOS:
```
C:\> rachel.exe
```

### In DOSBox:
```bash
dosbox
mount c /path/to/rachel-dos
c:
rachel.exe
```

## Controls

- **Arrow Keys**: Select card
- **Enter**: Play selected card
- **D**: Draw a card (when allowed)
- **ESC**: Return to menu

## Technical Details

- Uses BIOS interrupts for video control
- Direct CGA memory access at 0xB8000
- Supports all DOS versions from 3.3+
- Memory requirement: 256KB RAM
- Disk space: ~50KB

## Card Display

Cards are rendered using ASCII art and DOS box-drawing characters:
```
┌─────┐
│ A   │
│  ♠  │
│   A │
└─────┘
```

## Platform Notes

This implementation proves that Rachel can run on:
- Original IBM PC (1981)
- PC/XT with CGA card
- Any 286/386/486/Pentium DOS machine
- Modern systems via DOSBox
- FreeDOS on contemporary hardware

## The Sacred Rules

This implementation includes a copy of the canonical rules from [rachel-core](https://github.com/rachel-multiverse/rachel-core), ensuring identical gameplay across all platforms. The `rules.c` and `rules.h` files are copied directly to ensure the DOS build is completely self-contained.

## Status

Platform #002 of ∞ complete.
198 platforms to go...

## License

MIT - Because even DOS deserves Rachel.

---

*"640K ought to be enough for anybody to play Rachel"*