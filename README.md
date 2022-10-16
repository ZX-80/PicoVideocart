<div align="center">

# Videocart Pi

![badge](https://badgen.net/badge/version/v0.3.0/orange?style=flat-square)
![badge](https://badgen.net/badge/platform/RP2040/green?style=flat-square)
![badge](https://badgen.net/badge/Arduino/1.8.13/blue?style=flat-square)
![badge](https://badgen.net/badge/Arduino-Pico/2.5.2/blue?style=flat-square)

This [flash Videocart](https://en.wikipedia.org/wiki/Flash_cartridge), powered by a Raspberry Pi Pico, allows games to be loaded from an SD card and played on a real Fairchild Channel F. It supports both official games, and homebrew. This project is currently in development.
  
<p align = "center">

  <img width="550" src="https://user-images.githubusercontent.com/44975876/196056685-d7bb4b9e-c145-4a2e-935d-80e99a07aebd.png">
</p>

*\*Two Prototypes beside Videocart 12*
  
[Getting started](#getting-started) •
[Hardware](#hardware) •
[Firmware](#firmware)
  
</div>


# Getting Started

**Updating the firmware**

- The first thing you should do is update to the latest firmware. Instructions on how to do so can be found [here](Firmware#updating)

**Loading Games to the Flashcart**

- This flashcart uses an SD card for storage, which makes it convenient to add and removes games. It's important to note that the SD card must be formatted as either FAT16 or FAT32 (recommended). Once formatted, simply place your game files (should end in `.bin` or `.chf`) onto the SD card and insert it into the flashcart.

**Using the Multimenu**

- The multimenu allows a user to browse and select games from the SD card. Refer to [its repository](https://github.com/ZX-80/Multi-Menu) for more information.

# Hardware

The flashcart is essentially an emulator for an enhanced 3853 Static Memory Interface using SD cards instead of ROM ICs. As such, the PCB consists of an edge connector, a Raspberry Pi Pico (which handles most of the work), FRAM, and a few supporting chips for voltage level conversion (3.3V to/from 5V). The Pico was chosen due to its relatively low cost, high speed (428 MHz), large memory (264KB on-chip SRAM, 2MB on-board QSPI Flash), and because it has 2 cores.

Refer to the [schematics directory](Schematics) for more information.

## Videocart Case

These 3D files were provided to me by e5frog over at AtariAge

<div align = "center"> 
   <img width="80%" alt="image" src="https://user-images.githubusercontent.com/44975876/162847231-be1cd817-4aaf-4c2b-9cb1-2c3dfbe0539b.png">

   *3D Files (Bottom / Hatch / Top Lower / Top Upper)*
</div>

# Firmware

The firmware uses the [Arduino-Pico](https://github.com/earlephilhower/arduino-pico) core and, as mentioned above essentially emulates a 3853 Static Memory Interface IC, while adding a few extra features. Using data pulled from the SD card, it can emulate various types of memory (FRAM, ROM, SRAM, ...), ports (Timers, 1-bit SRAM, ...), and interrupts.

Refer to the [Firmware directory](Firmware) for more information.

