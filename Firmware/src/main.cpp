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
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWriteFast(LED_BUILTIN, HIGH);
    pinMode(LED_BOARD, OUTPUT);
    digitalWriteFast(LED_BOARD, HIGH);

    // Over/Under Clocking
    set_arm_clock(600000000);
    // Teensy 4.0: default 600MHz, max 1008 MHz (active cooling needed)
    // 528MHz may be ideal to increase lifespan according to: https://forum.pjrc.com/attachment.php?attachmentid=23160&d=1610141979
    // tempmonGetTemp() -> read internal temperature for testing
}

void loop() {
    while (!digitalReadFast(CHANNEL_F_BUS_PRESENCE)) {
        while(digitalReadFast(WRITE_PIN));

        // Falling edge
        dbus_mode(INPUT); // set dbus to input mode
            
        while(!digitalReadFast(WRITE_PIN)); 

        // Rising edge
        dbus = read_dbus();
        romc = read_romc();
        execute_romc();      // May set dbus to output mode
    }
}
