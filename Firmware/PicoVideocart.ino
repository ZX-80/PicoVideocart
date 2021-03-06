/*
 * Pico Videocart Firmware (rev 1A)
 *
 * This flash Videocart allows games to be loaded from an SD card and played
 * on a Fairchild Channel F.
 * 
 * Created April 2022 by Jefferson A. (3DMAZE at AtariAge)
 */

#include <pico/sem.h>
#include <pico/multicore.h>
#include <hardware/gpio.h>
#include <hardware/vreg.h>
#include <hardware/structs/sio.h>
#include <hardware/structs/iobank0.h>

#include "src/morse.h"
#include "src/ports.h"
#include "Videocart02.h"



constexpr uint8_t WRITE_PIN = 17;
constexpr uint8_t PHI_PIN = 26;
constexpr uint8_t DBUS0_PIN = 6;
constexpr uint8_t ROMC0_PIN = 18;
constexpr uint8_t DBUS_IN_CE_PIN = 15;
constexpr uint8_t DBUS_OUT_CE_PIN = 14;
                                                  // BIOS address space:      [0x00000 - 0x00800)
constexpr uint16_t VIDEOCART_START_ADDR = 0x800;  // Videocart address space: [0x00800 - 0x10000)
constexpr uint16_t VIDEOCART_SIZE = 0xF800;       // 62K



// GPIO functions //

/*! \brief Initialize a GPIO pin in input mode
 *
 * \param gpio GPIO number
 * \param out true for out, false for in
 * \param value If false clear the GPIO, otherwise set it.
 */
__force_inline void gpio_init_val(uint8_t gpio, bool out, bool value) {
    gpio_put(gpio, value);
    gpio_set_dir(gpio, out);
    gpio_set_function(gpio, GPIO_FUNC_SIO);
}

/*! \brief Get the ROMC bus value
 *
 * \return 5-bit ROMC bus value
 */
__force_inline uint8_t read_romc() {
    return (gpio_get_all() >> ROMC0_PIN) & 0x1F;
}

/*! \brief Get the data bus value
 *
 * \return 8-bit data bus value
 */
__force_inline uint8_t read_dbus() {
    return (gpio_get_all() >> DBUS0_PIN) & 0xFF;
}

void setup1() {

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
    
    // Overclock Pico to 428 MHz @ 1.3 V
    // 10 MHz <= system clock <= 426 MHz to 436 MHz
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    sleep_ms(1);
    if (!set_sys_clock_khz(428000, false)) { // Overclocking failed
        Morse::print("OCLK");
        panic("FATAL: Overclock returned unsuccessful");
    }
}



// Program ROM functions //

IO_port* IO[256];

enum class memory_t : uint8_t {
    rom,  // Default to read-only memory
    sram,
    fram,
    led
};
memory_t memory_type_LUT_base[VIDEOCART_SIZE];
memory_t *memory_type_LUT = &memory_type_LUT_base[-VIDEOCART_START_ADDR]; // Avoids the need to subtract VIDEOCART_START_ADDR from the address on each access

/*! \brief Get the content of the memory address in the program ROM
 *
 * \param address The location of the data
 * \return The content of the memory address
 */
__force_inline uint8_t read_program_byte(uint16_t address) {
    switch (memory_type_LUT[address]) {
        //case memory_t::fram:
            // TODO: implement FRAM read
        case memory_t::rom:
        case memory_t::sram:
        case memory_t::led:
            return videocart_memory[address];
    }
}

/*! \brief Set the content of the memory address in the program ROM
 *
 * \param address The location to write the data
 * \param data The byte to be written
 */
__force_inline void write_program_byte(uint16_t address, uint8_t data) {
    switch (memory_type_LUT[address]) {
        case memory_t::sram:
            videocart_memory[address] = data;
            break;
        //case memory_t::fram:
            // TODO: implement FRAM write
        case memory_t::led:
            gpio_xor_mask(1 << LED_BUILTIN); // Toggle LED
            break;
    }
}



// Core 1 loop //

uint8_t romc = 0x1C; // IDLE
uint8_t dbus;
uint16_t pc0;
uint16_t pc1;
uint16_t dc0;
uint16_t dc1;
uint16_t tmp;
uint8_t io_address;
IOPort* IOPorts[256];

/*! \brief Write a value to the data bus
 *
 * \param value The value to write
 * \param addr_source The address of the values source
 */
__force_inline void write_dbus(uint8_t value, uint16_t addr_source) {
    if (VIDEOCART_START_ADDR <= addr_source && addr_source < VIDEOCART_START_ADDR + VIDEOCART_SIZE) {
        gpio_put(DBUS_IN_CE_PIN, true);              // Disable input buffer
        gpio_clr_mask(0xFF << DBUS0_PIN);            // Write to DBUS
        gpio_set_mask((dbus = value) << DBUS0_PIN);
        gpio_set_dir_out_masked(0xFF << DBUS0_PIN);  // Set DBUS to output mode
        gpio_put(DBUS_OUT_CE_PIN, false);            // Enable output buffer
    }
}

/*! \brief Process ROMC instruction */
__force_inline void execute_romc() { 
    switch (romc) {
        case 0x00:
            /*
             * Instruction Fetch. The device whose address space includes the
             * contents of the PC0 register must place on the data bus the op
             * code addressed by PC0; then all devices increment the contents
             * of PC0.
             */
            write_dbus(read_program_byte(pc0), pc0);
            pc0 += 1;
            break;
        case 0x01:
            /*
             * The device whose address space includes the contents of the PC0
             * register must place on the data bus the contents of the memory
             * location addressed by PC0; then all devices add the 8-bit value
             * on the data bus as signed binary number to PC0.
             */
            write_dbus(read_program_byte(pc0), pc0);
            pc0 += (int8_t) dbus;
            break;
        case 0x02:
            /*
             * The device whose DC0 addresses a memory word within the address
             * space of that device must place on the data bus the contents of
             * the memory location addressed by DC0; then all devices increment
             * DC0.
             */
            write_dbus(read_program_byte(dc0), dc0);
            dc0 += 1;
            break;
        case 0x03:
            /*
             * Similiar to 0x00, except that it is used for immediate operands
             * fetches (using PC0) instead of instruction fetches.
             */
            write_dbus(io_address = read_program_byte(pc0), pc0);
            pc0 += 1;
            break;
        case 0x04:
            /*
             * Copy the contents of PC1 into PC0
             */
            pc0 = pc1;
            break;
        case 0x05:
            /*
             * Store the data bus contents into the memory location pointed
             * to by DC0; increment DC0.
             * 
             * CPU places "byte to be stored" on the dbus
             */
            write_program_byte(dc0, dbus);
            dc0 += 1;
            break;
        case 0x06:
            /*
             * Place the high order byte of DC0 on the data bus.
             * 
             * Note: Assumed to only apply to the device whose address space 
             * includes the contents of the DC0 register
             */
            write_dbus(dc0 >> 8, dc0);
            break;
        case 0x07:
            /*
             * Place the high order byte of PC1 on the data bus.
             * 
             * Note: Assumed to only apply to the device whose address space 
             * includes the contents of the PC1 register
             */
            write_dbus(pc1 >> 8, pc1);
            break;
        case 0x08:
            /*
             * All devices copy the contents of PC0 into PC1. The CPU outputs
             * zero on the data bus in this ROMC state. Load the data bus into
             * both halves of PC0, thus clearing the register.
             */
            pc1 = pc0;
            pc0 = (dbus << 8) | dbus;
            break;
        case 0x09:
            /*
             * The device whose address space includes the contents of the DC0
             * register must place the low order byte of DC0 onto the data bus.
             */
            write_dbus(dc0 & 0xff, dc0);
            break;
        case 0x0A:
            /*
             * All devices add the 8-bit value on the data bus, treated as
             * signed binary number, to the data counter.
             */
            dc0 += (int8_t) dbus;
            break;
        case 0x0B:
            /*
             * The device whose address space includes the value in PC1
             * must place the low order byte of PC1 onto the data bus.
             */
            write_dbus(pc1 & 0xff, pc1);
            break;
        case 0x0C:
            /*
             * The device whose address space includes the contents of the PC0
             * register must place the contents of the memory word addressed
             * by PC0 into the data bus; then all devices move the value that
             * has just been placed on the data bus into the low order byte of PC0.
             */
            write_dbus(read_program_byte(pc0), pc0);
            pc0 = (pc0 & 0xff00) | dbus;
            break;
        case 0x0D:
            /*
             * All devices store in PC1 the current contents of PC0, incremented
             * by 1; PC0 is unaltered.
             */
            pc1 = pc0 + 1;
            break;
        case 0x0E:
            /*
             * The device whose address space includes the contents of the PC0
             * register must place the word addressed by PC0 into the data bus.
             * The value on the data bus is then moved to the low order byte
             * of DC0 by all devices.
             */
            write_dbus(read_program_byte(pc0), pc0);
            dc0 = (dc0 & 0xff00) | dbus;
            break;
        case 0x0F:
            /*
             * The interrupting device with highest priority must place the
             * low order byte of the interrupt vector on the data bus.
             * All devices must copy the contents of PC0 into PC1. All devices
             * must move the contents of the data bus into the low order
             * byte of PC0.
             */
            // TODO
            pc1 = pc0;
            pc0 = (pc0 & 0xff00) | dbus;
            break;
        case 0x10:
            /*
             * Inhibit any modification to the interrupt priority logic.
             * 
             * Note: Also described as
             *   "PREVENT ADDRESS VECTOR CONFLICTS"
             *   "FREEZE INTERRUPT STATUS"
             *   "Place interrupt circuitry on an inhibit state that
             *   prevents altering the interrupt chain"
             *   "A NO-OP long cycle to allow time for the internal 
             *   priority chain to settle"
             *   "A NO-OP long cycle to allow time for the PRI IN/PRI
             *   OUT chain to settle"
             * in the Mostek F8 Data Book
             */
            break;
        case 0x11:
            /*
             * The device whose address space includes the contents of PC0
             * must place the contents of the addressed memory word on the
             * data bus. All devices must then move the contents of the
             * data bus to the upper byte of DC0.
             */
            write_dbus(read_program_byte(pc0), pc0);
            dc0 = (dc0 & 0x00ff) | (dbus << 8);
            break;
        case 0x12:
            /*
             * All devices copy the contents of PC0 into PC1. All devices then
             * move the contents of the data bus into the low order byte of PC0.
             */
            pc1 = pc0;
            pc0 = (pc0 & 0xff00) | dbus;
            break;
        case 0x13:
            /*
             * The interrupting device with highest priority must move the high
             * order half of the interrupt vector onto the data bus. All devices
             * must then move the contents of the data bus into the high order
             * byte of PC0. The interrupting device resets its interrupt circuitry
             * (so that it is no longer requesting CPU servicing and can respond
             * to another interrupt).
             */
            // TODO
            pc0 = (pc0 & 0x00ff) | (dbus << 8);
            break;
        case 0x14:
            /*
             * All devices move the contents of the data bus into the high
             * order byte of PC0.
             */
            pc0 = (pc0 & 0x00ff) | (dbus << 8);
            break;
        case 0x15:
            /*
             * All devices move the contents of the data bus into the high
             * order byte of PC1.
             */
            pc1 = (pc1 & 0x00ff) | (dbus << 8);
            break;
        case 0x16:
            /*
             * All devices move the contents of the data bus into the high
             * order byte of DC0.
             */
            dc0 = (dc0 & 0x00ff) | (dbus << 8);
            break;
        case 0x17:
            /*
             * All devices move the contents of the data bus into the low
             * order byte of PC0.
             */
            pc0 = (pc0 & 0xff00) | dbus;
            break;
        case 0x18:
            /*
             * All devices move the contents of the data bus into the low
             * order byte of PC1.
             */
            pc1 = (pc1 & 0xff00) | dbus;
            break;
        case 0x19:
            /*
             * All devices move the contents of the data bus into the low
             * order byte of DC0.
             */
            dc0 = (dc0 & 0xff00) | dbus;
            break;
        case 0x1A:
            /*
             * During the prior cycle, an I/O port timer or interrupt control
             * register was addressed; the device containing the addressed port
             * must place the contents of the data bus into the address port.
             */
            if (IOPorts[io_address] != nullptr) {
                IOPorts[io_address]->write(dbus);
            }
            break;
        case 0x1B:
            /*
             * During the prior cycle, the data bus specified the address of an
             * I/O port. The device containing the addressed I/O port must place
             * the contents of the I/O port on the data bus. (Note that the
             * contents of timer and interrupt control registers cannot be read
             * back onto the data bus).
             */
            if (IOPorts[io_address] != nullptr) {
                write_dbus(IOPorts[io_address]->read(), PROGRAM_START_ADDR);
            }
            break;
        case 0x1C:
            /*
             * None.
             *
             * Note: It's function is listed as IDLE in the Fairchild F3850 CPU
             * datasheet.
             * 
             * During OUTS/INS instructions in the range 2 to 15, the data bus
             * holds the address of an I/O port
             */
            io_address = dbus;
            break;
        case 0x1D:
            /*
             * Devices with DC0 and DC1 registers must switch registers.
             * Devices without a DC1 register perform no operation.
             */
            tmp = dc0;
            dc0 = dc1;
            dc1 = tmp;
            break;
        case 0x1E:
            /*
             * The devices whose address space includes the contents of PC0
             * must place the low order byte of PC0 onto the data bus.
             */
            write_dbus(pc0 & 0xff, pc0);
            break;
        case 0x1F:
            /*
             * The devices whose address space includes the contents of PC0
             * must place the high order byte of PC0 onto the data bus.
             */
            write_dbus((pc0 >> 8) & 0xff, pc0);
            break;
      }
}

void __not_in_flash_func(loop1)() {
    for (;;) {
        while(gpio_get(WRITE_PIN)==1) { //0 - 6 ns / 2.5 cycles latency
            tight_loop_contents();
        } 
        // Falling edge
        gpio_put(DBUS_OUT_CE_PIN, true);            // Disable output buffer
        gpio_set_dir_in_masked(0xFF << DBUS0_PIN);  // Set DBUS to input mode
        gpio_put(DBUS_IN_CE_PIN, false);            // Enable input buffer
            
        while(gpio_get(WRITE_PIN)==0) { //0 - 6 ns / 2.5 cycles latency
            tight_loop_contents();
        } 
        // Rising edge
        // Must complete within 1 us (1000 ns)
        dbus = read_dbus();
        romc = read_romc();
        execute_romc();
    }
}

// Running on core 0

#include <SPI.h>
#include <SD.h>

constexpr uint8_t serialClock = 2;
constexpr uint8_t transmit = 3;
constexpr uint8_t receive = 4;
constexpr uint8_t chipSelect = 5;

void setup() {

    // setup SD card pins
    SPI.setSCK(serialClock);
    SPI.setTX(transmit);
    SPI.setRX(receive);
    SPI.setCS(chipSelect);

    // Wait for a valid SD card   
    while (!SD.begin(chipSelect)) { // wait for SD card
        sleep_ms(500);
    }
    
    // Open the first file found on the SD card
    root = SD.open("/");
    do {
        File romFile = root.openNextFile()
    } while (romFile.isDirectory());

    if (romFile) {
        gpio_put(LED_BUILTIN, true); // Turn on LED to indicate success
        romFile.read((uint8_t*) (videocart_memory + 0x800), min(romFile.size(), VIDEOCART_SIZE - 1)); // Read up to 62K into program_rom
        romFile.close();

        // Setup default memory types
        memset(&memory_type_LUT[0x2800], memory_t::sram, 0x800); // 2K SRAM [0x2800 - 0x3000)
        memset(&memory_type_LUT[0x3800], memory_t::led, 0x800);  // 2K LED  [0x3800 - 0x4000)
        
        // Setup default ports
        IOPorts[0x24] = new Sram2102(0);
        IOPorts[0x25] = new Sram2102(1);
      
        // Setup experimental flashcart ports
        IOPorts[0x8] = new Random();
        IOPorts[0x9] = new HardwareStack();   // Data stack
        IOPorts[0xA] = new HardwareStack();   // Return stack
      
    } else {
        Morse::print("SD0");
        panic("FATAL: SD card is empty");
    }
};

void loop() {};
