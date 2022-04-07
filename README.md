<div align="center">

# PicoVideocart
 
![badge](https://badgen.net/badge/version/v0.9.0/orange?style=flat-square)
![badge](https://badgen.net/badge/platform/RP2040/green?style=flat-square)
![badge](https://badgen.net/badge/Arduino/1.8.13/blue?style=flat-square)

This [flash Videocart](https://en.wikipedia.org/wiki/Flash_cartridge) allows games to be loaded from an SD card and played on a real Fairchild Channel F. It's powered by a Raspberry Pi Pico, as well as two 74LVC245 ICs (5V -> 3.3V) and a 74HCT541 IC (3.3V -> 5V) for level conversion. This project is currently in development.

  
<p align = "center">
  <img width="33%" src="https://user-images.githubusercontent.com/44975876/160247541-6484ebdc-2509-400d-8f8a-3ad9957b1618.png">
</p>

*\*PCB KiCad mock-up*
  
[Getting started](#getting-started) •
[PCB](#pcb) •
[Videocart Case](#videocart-case) •
[Raspberry Pi Pico Code](#raspberry-pi-pico-code)
  
</div>

# Getting Started

# PCB

# Videocart Case

# Raspberry Pi Pico Code

This project uses arduino-pico v1.13.1 by Earle Philhower. Installation instructions and documentation can be found on his repo: https://github.com/earlephilhower/arduino-pico

Currently it works in a testing environment (hooked up to an Arduino Due simulating a channel F), but does not work on a real unit.

### Project Code Tasks

- [X] ~~Pass all ROMC tests on the Arduino Due~~
- [ ] Work on a real Channel F with the game hardcoded into the Raspberry Pi Pico
- [ ] Work on a real Channel F with the game loaded from an SD Card (hardcoded filename)
- [ ] Work on a real Channel F with the game loaded from an SD Card (selected from a menu)
- [ ] 1-bit SRAM support on ports 24/25
- [X] ~~SRAM support at addresses 0x2800 - 0x2FFF~~
- [X] ~~LED support at addresses 0x3800 - 0x38FF~~
- [ ] Lowered power consumption to allow running the Pico from the Channel F bus
- [ ] Interrupt support
