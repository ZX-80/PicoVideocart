<div align="center">

# PicoVideocart
 
![badge](https://badgen.net/badge/version/v0.1.0/orange?style=flat-square)
![badge](https://badgen.net/badge/platform/Teensy4.0/green?style=flat-square)
![badge](https://badgen.net/badge/Teensyduino/1.58/blue?style=flat-square)

This [flash Videocart](https://en.wikipedia.org/wiki/Flash_cartridge), powered by a Teensy 4.0, allows games to be loaded from an SD card and played on a real Fairchild Channel F. It supports both official games, and homebrew. This project is currently in development.
  
<p align = "center">
  <img width="550" src="https://user-images.githubusercontent.com/44975876/196056685-d7bb4b9e-c145-4a2e-935d-80e99a07aebd.png">
</p>

*\*Two Prototypes beside Videocart 12*
  
[Getting started](#getting-started) •
[Hardware](#hardware) •
[Firmware](#firmware) •
[Building](#building)
  
</div>

# Getting Started

**Updating the firmware**

- The first thing you should do is update to the latest firmware:
  - Download the firmware file from [here](https://github.com/ZX-80/PicoVideocart/releases/latest/download/videocart_firmware.hex)
  - Put it on the SD card, or upload it via USB
  - Power up the Flashcart, and it will install the new firmware
  - When it's done, the LED will blink

> [!WARNING] 
> Do not power down the device while it's updating. If you do, it may corrupt the firmware. If this happens, you must use the [Teensy loader](https://www.pjrc.com/teensy/loader.html) to upload
> the firmware file instead.

**Loading Games to the Flashcart**

- This flashcart uses an SD card for storage, which makes it convenient to add and removes games. It's important to note that the SD card must be formatted as either FAT16 or FAT32 (recommended). Once formatted, simply place your game files (should end in `.bin` or `.chf`) onto the SD card and insert it into the flashcart.
- Alternatively, you can connect the flashcart to your PC via a micro-usb cable. It will then appear as an MTP (Media Transfer Protocol) device, similar to a phone. Then games can be removed or added without removal of the SD card.

**Using the Multimenu**

- The multimenu allows a user to browse and select games from the SD card. Refer to [its repository](https://github.com/ZX-80/Multi-Menu) for more information.

# Hardware

For information on the PCB, schematic, or 3D case, visit [this page](Hardware).

# Firmware

For information on the firmware, such as planned features, and build instructions, visit [this page](Firmware).

# Building

You will need Visual Studio Code, with the Platform IO extension installed. Then it should be a simple case of downloading this repository, and building with:

```cmd
pio run --target upload
```