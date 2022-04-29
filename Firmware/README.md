As mentioned, the firmware essentially emulates a 3853 Static Memory Interface IC, while adding a few extra features. These include things such as an SD card multi-menu, FRAM, and a few extra IO ports. 

# 🥅 Project Goals

**Game Support**
- [ ] Work with all official games (not including special ICs)
- [ ] Work with all official games (including special ICs)

**SD Card Support**
- [X] ~~Pass all ROMC tests on the Arduino Due~~
- [X] ~~Work on a real Channel F with the game hardcoded into the Raspberry Pi Pico~~
- [X] ~~`.bin` file format support~~
- [X] ~~Work on a real Channel F with the game loaded from an SD Card (hardcoded filename)~~
- [ ] Work on a real Channel F with the game loaded from an SD Card (selected from a menu)
- [ ] `.chf` file format support

**Special ICs**
- [X] ~~SRAM support at addresses 0x2800 - 0x2FFF~~
- [X] ~~LED support at addresses 0x3800 - 0x38FF~~
- [ ] FRAM support
- [ ] I/O ports
  - [ ] 1-bit SRAM support [0x18/0x19]
  - [ ] Emulate 3853 SMI ports
    - [ ] programmable interrupt vector address [0x0C/0x0D]
    - [ ] programmable timer [0x0E]
    - [ ] interrupt control [0x0F]

**Other**
- [ ] Lowered power consumption to allow running the Pico from the Channel F bus (70 mA max)

# ⚙️ Setup

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
