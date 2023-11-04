/** \file main.cpp
 *
 * \brief Videocart Firmware (rev 3A)
 * 
 * \details A Videocart that allows games to be loaded from an SD card and
 * played on a Fairchild Channel F.
 * 
 * Created 2023 by 3DMAZE @ AtariAge
 * 
 * ### Limitations
 * 
 * To save space in program_attribute, chip type is assumed to be 8-bit
 * value. This shouldn't be a problem until there are more than 256 chip 
 * types defined in the standard.
 */

// TODO: Integrate FlasherX:   https://github.com/joepasquariello/FlasherX
// TODO: Integrate MTP_Teensy: https://github.com/KurtE/MTP_Teensy/tree/main
// TODO: SD card (load bin, load chf, write protect, card detect, dma, multi-menu)
// TODO: FRAM (chf can map it to memory. It will be written back to the game file on SD (dirty bit). DMA)
// TODO: Threads?: https://github.com/ftrias/TeensyThreads
// TODO: Development features
// TODO: IO Ports (Stack, music/arm co-processor, rng, MK3870, 3853)
// TODO: multimenu (multiple files on screen, launch game without reset)
// NOTE: Could a CPLD help?

#include <Arduino.h>

#include "romc.hpp"

extern "C" uint32_t set_arm_clock(uint32_t frequency);

void setup() {

    // Initialize data bus pins
    pinMode(DBUS0_PIN, INPUT);
    pinMode(DBUS1_PIN, INPUT);
    pinMode(DBUS2_PIN, INPUT);
    pinMode(DBUS3_PIN, INPUT);
    pinMode(DBUS4_PIN, INPUT);
    pinMode(DBUS5_PIN, INPUT);
    pinMode(DBUS6_PIN, INPUT);
    pinMode(DBUS7_PIN, INPUT);

    // Initialize data bus level shifters
    pinMode(DBUS_MODE_PIN, OUTPUT);
    dbus_mode(INPUT);
    
    // Initialize ROMC pins
    pinMode(ROMC0_PIN, INPUT);
    pinMode(ROMC1_PIN, INPUT);
    pinMode(ROMC2_PIN, INPUT);
    pinMode(ROMC3_PIN, INPUT);
    pinMode(ROMC4_PIN, INPUT);

    // Initialize other pins
    pinMode(WRITE_PIN, INPUT);
    pinMode(PHI_PIN, INPUT);
    pinMode(CHANNEL_F_BUS_PRESENCE, INPUT);
 
    // Initialize LEDs
    pinMode(LED_BOARD, OUTPUT);
    digitalWriteFast(LED_BOARD, HIGH);

    // Over/Under Clocking
    set_arm_clock(600000000);
    // Teensy 4.0: default 600MHz, max 1008 MHz (active cooling needed)
    // 528MHz may be ideal to increase lifespan according to: https://forum.pjrc.com/attachment.php?attachmentid=23160&d=1610141979
    // tempmonGetTemp() -> read internal temperature for testing

    memset(program_attribute, RESERVED_CT::id, 0x800); // FIXME: temporary fix until loader code is added
}

// NOTE: Moving loop to ISR triggered by WRITE_PIN rising edge could free up the CPU when nothing's happening

void loop() {

    // while (!digitalReadFast(CHANNEL_F_BUS_PRESENCE)) {
        // Wait for rising edge
        while(!digitalReadFast(WRITE_PIN)); 
        dbus = read_dbus();
        romc = read_romc();
        execute_romc();      // May set dbus to output mode

        // Wait for falling edge
        while(digitalReadFast(WRITE_PIN));
        dbus_mode(INPUT); // set dbus to input mode
    // }
}
