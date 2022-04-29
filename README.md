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

- This flashcart uses a micro SD card for storage, which makes it convenient to add and removes games. However, the micro SD card must be formatted as either FAT16 or FAT32 (recommended). Once formatted, simply place your game files (should end in `.bin`) onto the SD card and insert it into the flashcart.

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

This project uses arduino-pico v1.13.1 by Earle Philhower. Installation instructions and documentation can be found on his repo: https://github.com/earlephilhower/arduino-pico

The board configuration is as follows:
- **Selected Board:** Generic RP2040 (Raspberry Pi Pico/RP2040)
- **Flash Size:** 2MB (No FS)
- **CPU Speed:** 125 MHz
- **Optimize:** Fast (-Ofast) (maybe slower)
- **RTTI:** Disabled
- **Debug Port:** Disabled
- **Debug Level:** None
- **USB Stack:** Pico SDK
- **Boot Stage 2:** W25Q080 QSPI/4 \*

\* *Sets the flash clock divider to 4, which allows the flash to operate up to 532 MHz. This is required when overclocking the Raspberry Pi Pico to 428 MHz*

### Project Goals

- [X] ~~Pass all ROMC tests on the Arduino Due~~
- [X] ~~Work on a real Channel F with the game hardcoded into the Raspberry Pi Pico~~
- [X] ~~Work on a real Channel F with the game loaded from an SD Card (hardcoded filename)~~
- [ ] Work on a real Channel F with the game loaded from an SD Card (selected from a menu)
- [X] ~~SRAM support at addresses 0x2800 - 0x2FFF~~
- [X] ~~LED support at addresses 0x3800 - 0x38FF~~
- [ ] Lowered power consumption to allow running the Pico from the Channel F bus (70 mA max)
- [ ] I/O ports
  - [ ] 1-bit SRAM support [0x18/0x19]
  - [ ] Emulate 3853 SMI ports
    - [ ] programmable interrupt vector address [0x0C/0x0D]
    - [ ] programmable timer [0x0E]
    - [ ] interrupt control [0x0F]


# Images

<div align = "center">
  <img width="90%" src="https://user-images.githubusercontent.com/44975876/163595785-561fc3ba-3969-4d96-9882-f828548ac4f7.png">

  *A demonstration of CCtro.bin (by [Frog](https://www.youtube.com/watch?v=X0mVxEY4aD0)) & boxingv1.bin (by Mikebloke from AtariAge) running on a real Channel F (graphical glitches are from a bad Channel F, not the flashcart)*
 
  <img width="90%" src="https://user-images.githubusercontent.com/44975876/163596874-a31e1824-4e23-4a60-8353-af2ef9235165.png">
 
  *A demonstration of Videocart 2 running off the flashcart on a real Channel F (graphical glitches are from a bad Channel F, not the flashcart)*

</div>
