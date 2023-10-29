<div align="center">

# Hardware Design

The flashcart is essentially an emulator for an enhanced 3853 Static Memory Interface using SD cards instead of ROM ICs.

[Revision 3A](#revision-3a) •
[Revision 2A](#revision-2a) •
[Revision 1A](#revision-1a) •
[3D Case](#3d-case)
  
</div>

## Revision 3A

This is the current PCB revision. 

The following major changes were made:

- All DIP componants were replaced with SMT parts
- The Raspberry Pi Pico was replaced with the faster Teensy 4.0
- 5V to 3.3V level shifters were replaced with thier bus hold variants
- ICs were moved higher and to a single side up to avoid interfereing with the connector slot
- An LED was added to the board, as the internal one can't be used with SPI

It consists of 4 main groups

- **The microcontroler**
  - 1x Teensy 4.0
  - 2x 1N5817 Schottky diode (used to power the flashcart from the USB/Channel F's 5V)
- **The bus interface** 
  - 2x 74LVCH245A IC (5V to 3.3V level shifter)
  - 1x 74HCT541 IC (3.3V to 5V level shifter)
  - 1x NPN transistor switching circuit (used to control the 5V open-collector `INTRQ` pin from a 3.3V GPIO pin)
  - 1x 74LVC2G14 IC (NOT gate for the data bus IC chip enables)
- **The SD card interface**
- **The non-volatile memory**
  - 1x FM25V10 IC (1Mbit FRAM)

<div align = "center">
   <img width="80%" src="https://github.com/ZX-80/PicoVideocart/assets/44975876/44f3a7ff-46b0-4c8f-84b2-5b062c5b5eac">

   *Schematic*
</div>

## Revision 2A

The following major changes were made:

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

## Videocart Case

These 3D files were provided to me by e5frog over at AtariAge

<div align = "center"> 
   <img width="80%" alt="image" src="https://user-images.githubusercontent.com/44975876/162847231-be1cd817-4aaf-4c2b-9cb1-2c3dfbe0539b.png">
 
   *3D Files (Bottom / Hatch / Top Lower / Top Upper)*
</div>


