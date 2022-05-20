<div align="center">

# PicoVideocart
 
![badge](https://badgen.net/badge/version/v0.9.3/orange?style=flat-square)
![badge](https://badgen.net/badge/platform/RP2040/green?style=flat-square)
![badge](https://badgen.net/badge/Arduino/1.8.13/blue?style=flat-square)

The Pico Videocart is an open source [flash cartridge](https://en.wikipedia.org/wiki/Flash_cartridge) for the Fairchild Channel F. It's powered by the Raspberry Pi Pico, and allows games to be selected, and loaded from an SD card, using a multimenu. It supports both official games, and homebrew. This project is currently in development.

  
<p align = "center">
  <img width="33%" src="https://user-images.githubusercontent.com/44975876/160247541-6484ebdc-2509-400d-8f8a-3ad9957b1618.png">
</p>

*\*PCB KiCad mock-up*
  
[Getting started](#getting-started) •
[Hardware](#hardware) •
[Firmware](#firmware) •
[Images](#images)
  
</div>

# Getting Started

**Loading Games to the Flashcart**

- This flashcart uses an SD card for storage, which makes it convenient to add and removes games. It's important to note that the SD card must be formatted as either FAT16 or FAT32 (recommended). Once formatted, simply place your game files (should end in `.bin` or `.chf`) onto the SD card and insert it into the flashcart.

**Using the Multimenu**

- The multimenu is still in development at this time

# Hardware

The flashcart is essentially an emulator for an enhanced 3853 Static Memory Interface using SD cards instead of ROM ICs. A Raspberry Pi Pico was chosen due to its relativly low cost, high speed (428 MHz), large memory (264KB on-chip SRAM, 2MB on-board QSPI Flash), and because it has 2 cores.

More information (and schematics) can be found in the [Schematics directory](Schematics)

## Videocart Case

These 3D files were provided to me by e5frog over at AtariAge

<div align = "center"> 
   <img width="80%" alt="image" src="https://user-images.githubusercontent.com/44975876/162847231-be1cd817-4aaf-4c2b-9cb1-2c3dfbe0539b.png">
 
   *3D Files (Bottom / Hatch / Top Lower / Top Upper)*
</div>

# Firmware

As mentioned, the firmware essentially emulates a 3853 Static Memory Interface IC, while adding a few extra features. Such as:
- An SD card multi-menu
- FRAM (for high scores and saves)
- SRAM (R/W memory)
- An LED (controlled in software)
- Extra IO ports
  - Timers
  - 1-bit SRAM
  - Interrupts

More information (and the firmware) can be found in the [Firmware directory](Firmware)

# Images

<div align = "center">
  <img width="90%" src="https://user-images.githubusercontent.com/44975876/163595785-561fc3ba-3969-4d96-9882-f828548ac4f7.png">

  *A demonstration of CCtro.bin (by [Frog](https://www.youtube.com/watch?v=X0mVxEY4aD0)) & boxingv1.bin (by Mikebloke from AtariAge) running on a real Channel F (graphical glitches are from a bad Channel F, not the flashcart)*
</div>
