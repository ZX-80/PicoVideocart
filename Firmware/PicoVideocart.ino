/** \file PicoVideocart.ino
 *
 * \brief Pico Videocart Firmware (rev 2A)
 * 
 * \details A Videocart that allows games to be loaded from an SD card and
 * played on a Fairchild Channel F.
 * 
 * Created 2022 by 3DMAZE @ AtariAge
 * 
 * ### Limitations
 * 
 * To save space in both program_attribute and ChipTypes, chip_type is assumed to be an 8-bit value.
 * This shouldn't be a problem until there are more than 256 chip types defined in the standard.
 */

// TODO: read/write memory according to program_attribute
// TODO: Disconnecting when loading
// TODO: Minor menu work (remove ".bin", use .chf title, reload menu when holding reset, etc.) 
// TODO: Special char support 
// TODO: set ports according to hardware type
// TODO: Directories
// TODO: Double reset issue
// TODO: Cache(?) issue

#include "loader.hpp"
#include "romc.hpp"

#include <SPI.h>
#include <SD.h>

#include <pico/sem.h>
#include <pico/stdlib.h>               // Overclocking functions
#include <pico/multicore.h>            // Allow code to be run on both cores
#include <hardware/gpio.h>
#include <hardware/vreg.h>             // Voltage control for overclocking
#include <hardware/structs/sio.h>
#include <hardware/structs/iobank0.h>
#include <hardware/structs/xip_ctrl.h>
#include <hardware/structs/bus_ctrl.h>

void setup1() { // Core 1
    // Set Core 1 priority to high
    bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_PROC1_BITS;

    // Initialize data bus pins
    gpio_set_dir_in_masked(0xFF << DBUS0_PIN);        // Set DBUS to input mode
    gpio_clr_mask(0xFF << DBUS0_PIN);                 // Set DBUS data to 0
    gpio_set_function(DBUS0_PIN + 0, GPIO_FUNC_SIO);  // Set DBUS pins to software controlled
    gpio_set_function(DBUS0_PIN + 1, GPIO_FUNC_SIO);
    gpio_set_function(DBUS0_PIN + 2, GPIO_FUNC_SIO);
    gpio_set_function(DBUS0_PIN + 3, GPIO_FUNC_SIO);
    gpio_set_function(DBUS0_PIN + 4, GPIO_FUNC_SIO);
    gpio_set_function(DBUS0_PIN + 5, GPIO_FUNC_SIO);
    gpio_set_function(DBUS0_PIN + 6, GPIO_FUNC_SIO);
    gpio_set_function(DBUS0_PIN + 7, GPIO_FUNC_SIO);
    
    // Initialize ROMC pins
    gpio_set_dir_in_masked(0x1F << ROMC0_PIN);        // Set ROMC to input mode
    gpio_clr_mask(0x1F << ROMC0_PIN);                 // Set ROMC data to 0
    gpio_set_function(ROMC0_PIN + 0, GPIO_FUNC_SIO);  // Set ROMC pins to software controlled
    gpio_set_function(ROMC0_PIN + 1, GPIO_FUNC_SIO);
    gpio_set_function(ROMC0_PIN + 2, GPIO_FUNC_SIO);
    gpio_set_function(ROMC0_PIN + 3, GPIO_FUNC_SIO);
    gpio_set_function(ROMC0_PIN + 4, GPIO_FUNC_SIO);
 
    // Initialize other cartridge pins
    gpio_init_val(WRITE_PIN, GPIO_IN, false);
    gpio_init_val(PHI_PIN, GPIO_IN, false);
    gpio_init_val(DBUS_OUT_CE_PIN, GPIO_OUT, true);
    gpio_init_val(DBUS_IN_CE_PIN, GPIO_OUT, false);
    gpio_init_val(LED_BUILTIN, GPIO_OUT, true);

   // Shift into maximum overdrive (aka 400 MHz @ 1.3 V)
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    sleep_ms(1);
    if (!set_sys_clock_khz(400000, false)) { // 428000 is known to work on some devices
        blink_code(BLINK::OVERCLOCK_FAILED);
        panic("Overclock was unsuccessful");
    }
}

void __not_in_flash_func(loop1)() { // Core 1
    for (;;) {
        while(gpio_get(WRITE_PIN)==1) {
            tight_loop_contents();
        } 
        // Falling edge
        gpio_put(DBUS_OUT_CE_PIN, true);            // Disable output buffer
        gpio_set_dir_in_masked(0xFF << DBUS0_PIN);  // Set DBUS to input mode
        gpio_put(DBUS_IN_CE_PIN, false);            // Enable input buffer
            
        while(gpio_get(WRITE_PIN)==0) {
            tight_loop_contents();
        } 
        // Rising edge
        dbus = read_dbus();
        romc = read_romc();
        execute_romc();
    }
}

void __not_in_flash_func(setup)() { // Core 0

    // Setup SD card pins
    SPI.setSCK(SERIAL_CLOCK_PIN);
    SPI.setTX(TRANSMIT_PIN);
    SPI.setRX(RECEIVE_PIN);
    SPI.setCS(SD_CARD_CHIP_SELECT_PIN);
    gpio_init_val(WRITE_PROTECT_PIN, GPIO_IN, false);
    gpio_pull_up(WRITE_PROTECT_PIN);
    gpio_init_val(FRAM_CHIP_SELECT_PIN, GPIO_OUT, true);

    // Load the game
    //FIXME: SD.begin(SD_CARD_CHIP_SELECT_PIN, SD_SCK_MHZ(50))
    // fall back to 25, then 4 if it doesn't work (but only if there's an SD inserted)
    while (!SD.begin(SD_CARD_CHIP_SELECT_PIN)) { // wait for SD card
        sleep_ms(250);
    }
    File romFile = SD.open("boot.bin");
    load_game(romFile);

    uint16_t file_counter = 0;
    File dir = SD.open("/");
    File current_file = dir.openNextFile();
    while (current_file) {
        if (current_file.isDirectory()) {
            file_data[file_counter].title[0] = '/';
        } else {
            file_data[file_counter].title[0] = ' ';
        }
        file_data[file_counter].isFile = !current_file.isDirectory();
        string_copy((char*) file_data[file_counter].title + 1, (char*)current_file.name(), 30, true, '\0');
        file_counter++;
        current_file = dir.openNextFile();
    }
    DIR_LIMIT = file_counter;
};

void __not_in_flash_func(loop)() { // Core 0
    // Re-run setup if SD card inserted
    // FIXME: make a SD_DETECT function in gpio.hpp
    sleep_ms(250);
    if (gpio_get(WRITE_PROTECT_PIN) != old_write_protect) {
        if (old_write_protect) {
            sleep_ms(250);
            if (gpio_get(WRITE_PROTECT_PIN) == 0) {
                SD.end();
                setup();
            }
        }
        old_write_protect = gpio_get(WRITE_PROTECT_PIN);
    }

    if (load_new_game_trigger) {
        load_new_game_trigger = false;
        while (pc0 >= 0x800) {
            ; // We need to wait until the menu has jumped to 0 before disconnecting
        }

        // Get file from index
        File romFile;
        File dir = SD.open("/");
        dir.rewindDirectory();
        uint16_t file_index = static_cast<Launcher*>(IOPorts[0xFF])->file_index;
        for (uint32_t i = 0; i <= file_index; i++) {
            romFile = dir.openNextFile();
        }
        load_game(romFile);
    }
};
