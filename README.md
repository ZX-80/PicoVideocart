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

## Revision 1A

<div align = "center">
   <img width="80%" alt="image" src="https://user-images.githubusercontent.com/44975876/162845450-59c9edab-7b7b-4587-b028-241cebfad19d.png">
 
   *3D View (Front / Back)*
</div>

<div align = "center">
   <img width="80%" alt="image" src="https://user-images.githubusercontent.com/44975876/162845355-da4104ff-fc61-4df0-86b5-71c5d07db3e8.png">
 
   *Schematic*
</div>

### BOM

| Designator | Designation | Package                                                     | Quantity | Notes                                                        |
| ---------- | ----------- | ----------------------------------------------------------- | -------- | ------------------------------------------------------------ |
| Q1         | 2N3904      | TO-92 <br />Inline                                          | 1        |                                                              |
| U1         | 74HCT541    | DIP-20<br />W7.62mm                                         | 1        | 3.3V --> 5V conversion                                       |
| U2-3       | 74LVC245    | DIP-20<br />W7.62mm                                         | 2        | 5V --> 3.3V conversion                                       |
| U4         | RPi Pico    | RPi Pico SMD TH                                             | 1        |                                                              |
| C1-3       | 0.1uF       | C_Disc<br />D9.0mm x W2.5mm x P5.00mm                       | 3        |                                                              |
| R1-4       | 10K         | R Axial DIN 0207 Horizontal<br />L6.3mm x D2.5mm x P10.16mm | 4        |                                                              |
| D1         | D Schottky  | D DO-15 Horizontal<br />P12.70mm                            | 1        |                                                              |
|            | ADA254      | 1x08 BIG                                                    | 1        | Micro SD Card breakout                                       |
| J1         | Pin Headers | PinHeader 1x5 Horizontal<br />P2.54mm                       | 1        | A direct connection to the Pico's SWD Interface and Serial Interface (UART0) for debugging |


# Videocart Case

These 3D files were provided to me by e5frog over at AtariAge

<div align = "center">
   <img width="80%" alt="image" src="https://user-images.githubusercontent.com/44975876/162847231-be1cd817-4aaf-4c2b-9cb1-2c3dfbe0539b.png">
 
   *3D Files (Bottom / Hatch / Top Lower / Top Upper)*
</div>

# Raspberry Pi Pico Code

This project uses arduino-pico v1.13.1 by Earle Philhower. Installation instructions and documentation can be found on his repo: https://github.com/earlephilhower/arduino-pico

Currently it works in a testing environment (hooked up to an Arduino Due simulating a channel F), but does not work on a real unit.

### Project Code Goals

- [X] ~~Pass all ROMC tests on the Arduino Due~~
- [ ] Work on a real Channel F with the game hardcoded into the Raspberry Pi Pico
- [ ] Work on a real Channel F with the game loaded from an SD Card (hardcoded filename)
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
