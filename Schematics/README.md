<div align="center">

# Hardware Design

The flashcart is essentially an emulator for an enhanced 3853 Static Memory Interface using SD cards instead of ROM ICs. A Raspberry Pi Pico was chosen due to its relativly low cost, high speed (428 MHz), large memory (264KB on-chip SRAM, 2MB on-board QSPI Flash), and because it has 2 cores.

[Revision 1A](#revision-1a) •
[Revision 2A](#revision-2a) •
[Photos](#photos)
  
</div>

## Revision 1A

This is the first prototype PCB. It makes heavy use of DIP componants and breakout boards to simplify manufacturing.

It consists of 3 main groups

- **The microcontroler**
  - 1x Raspberry Pi Pico
  - 1x 1N5817 Schottky diode (used to power the flashcart from the Channel F's 5V)
- **The bus interface** 
  - 2x 74LVC245 IC (5V to 3.3V level shifter)
  - 1x 74HCT541 IC (3.3V to 5V level shifter)
  - 1x NPN transistor switching circuit (used to control the 5V open-collector `INTRQ` pin from a 3.3V GPIO pin)
- **The SD card interface**
  - 1x ADA254 (micro-SD breakout)

<div align = "center">
   <img width="80%" alt="image" src="https://user-images.githubusercontent.com/44975876/162845355-da4104ff-fc61-4df0-86b5-71c5d07db3e8.png">
 
   *Schematic*
</div>

## Revision 2A

This is the current PCB revision. The following major changes were made:

- Diode changed to **1N5817RLG** 
- Removed the debug connector
- SD card replaces micro-SD breakout
  - Includes write protect
- 256K FRAM SPI added
- Edge connector 
  - Properly aligned
  - Height doubled

It consists of 4 main groups

- **The microcontroler**
  - 1x Raspberry Pi Pico
  - 1x 1N5817 Schottky diode (used to power the flashcart from the Channel F's 5V)
- **The bus interface** 
  - 2x 74LVC245 IC (5V to 3.3V level shifter)
  - 1x 74HCT541 IC (3.3V to 5V level shifter)
  - 1x NPN transistor switching circuit (used to control the 5V open-collector `INTRQ` pin from a 3.3V GPIO pin)
- **The SD card interface**
- **The non-volatile memory**
  - 1x FM25V10 IC (1Mbit FRAM)

<div align = "center">
   <img width="80%" src="https://user-images.githubusercontent.com/44975876/165879449-62209240-8a97-4139-bec5-1ae0a87f5627.png">
 
   *Schematic*
</div>

# Photos

<div align = "center">
   <img width="90%" src="https://user-images.githubusercontent.com/44975876/163595652-a342427b-8f57-4a17-bc1d-d46ca3924447.png">

   *A partially constructed board (missing SD card, interrupt components, and 5V diode) with the 3D printed case*
  
</div>
