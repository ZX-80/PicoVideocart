# Features 🛰️

As mentioned, the firmware essentially emulates a 3853 SMI & EEPROM IC, while adding a few extra features

- [X] Support for all official Videocarts
- [X] Loading files from an SD card
- [X] `.bin` file format support
- [ ] `.chf` file format support
- [ ] SD card multi-menu
- [ ] Save game / highscore support using an FRAM IC

**Memory-mapped devices**

- [X] ROM
- [x] LED
- [x] SRAM
- [ ] FRAM

**Port-mapped devices**

- [X] 1-bit 2102 SRAM
- [X] Random number generator
- [X] Data/Return stacks
- [ ] Programmable interrupt vector address
- [ ] Programmable timer
- [ ] Interrupt control

# Setup ⚙️

To modify and flash the firmware, you'll need to do the following:

**Required installation**
- Install [Arduino IDE 1.x.x](https://www.arduino.cc/en/software)
- Install [VS code](https://code.visualstudio.com/)
- Install the Arduino extension (published by Microsoft) for VS code
- Install arduino-pico v1.13.1 by Earle Philhower
  - Installation instructions and documentation can be found on his repo: https://github.com/earlephilhower/arduino-pico

**Open the Firmware and Connect the Board**
- Download the firmware (from this directory) for your boards revision
- Open the `.ino` file in VS code
- Plug your board into a USB port on your computer

**Configure VS Code**
- Set the programmer as `rp2040:atmel_ice`
- Set the board configuration as follows:
  - **Selected Board:** Generic RP2040 (Raspberry Pi Pico/RP2040)
  - **Flash Size:** 2MB (No FS)
  - **CPU Speed:** 125 MHz
  - **Optimize:** Fast (-Ofast) (maybe slower)
  - **RTTI:** Disabled
  - **Debug Port:** Disabled
  - **Debug Level:** None
  - **USB Stack:** Pico SDK
  - **Boot Stage 2:** W25Q080 QSPI/4
    - *Sets the flash clock divider to 4, which allows the flash to operate up to 532 MHz. This is required when overclocking the Raspberry Pi Pico to 428 MHz*
-  Select the correct serial port

You should now be able to modify the firmware and flash it to your device
