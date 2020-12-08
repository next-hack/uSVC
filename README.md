# uSVC
uChip Simple VGA Console
An open source, easy to build retro game console!

This project is a very simple VGA console, based on uChip. Games are loaded from the SD card, and you can use standard keyboards or gamepads.

The console is made of few, easy to find and easy to solder components. There is only one SMD component: the micro-SD card reader.

The ATSAMD21 (A 48 MHz Cortex M0+) generates the VGA and audio signals, and provides up to 256 simultaneous colors. There are 3 video modes:
- Bitmapped Mode
- 8 bpp tiled mode
- 4 bpp tiled mode.

Each mode has its own peculiarity. For more information, visit next-hack.com!

You might also want to take a look at:

- The game editor source code: https://github.com/next-hack/uChipGameMapEditor
- Redballs, a Worms-like game: https://github.com/next-hack/uSVC_Redballs
- Fairplay race: https://github.com/next-hack/uSVC_Fairplay-race
- uSVC Tetris: https://github.com/next-hack/uSVC_Tetris
