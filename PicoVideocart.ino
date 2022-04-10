#include <hardware/structs/iobank0.h>
#include "Videocart12.h"
#include "pico/multicore.h"
#include "pico/sem.h"

constexpr uint8_t WRITE_PIN = 17;
constexpr uint8_t PHI_PIN = 26;
constexpr uint8_t DBUS0_PIN = 6;
constexpr uint8_t ROMC0_PIN = 18;
constexpr uint8_t DBUS_IN_CE_PIN = 15;
constexpr uint8_t DBUS_OUT_CE_PIN = 14;

constexpr uint16_t PROGRAM_START_ADDR = 0x800;  // Program address space: [0x0800 - 0x10000)
constexpr uint16_t SRAM_START_ADDR = 0x2800;    // SRAM address space: [0x2800 - 0x3000)
constexpr uint16_t SRAM_SIZE = 0x800;           // 2K

semaphore_t debugCoreSemaphore;


// GPIO functions //

inline void gpio_init(uint8_t gpio, bool direction, bool value) __attribute__((always_inline));
inline void gpio_init(uint8_t gpio, bool direction, bool value) {
    gpio_put(gpio, value);
    gpio_set_dir(gpio, direction);
    gpio_set_function(gpio, GPIO_FUNC_SIO);
}

inline uint8_t read_romc() __attribute__((always_inline));
inline uint8_t read_romc() {
    return (gpio_get_all() >> ROMC0_PIN) & 0x1F;
}

inline uint8_t read_dbus() __attribute__((always_inline));
inline uint8_t read_dbus() {
    return (gpio_get_all() >> DBUS0_PIN) & 0xFF;
}

void setup1() {
    
    // Initialize debug semaphore
    sem_init(&debugCoreSemaphore, 0, 1);

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
    gpio_init(WRITE_PIN, GPIO_IN, false);
    gpio_init(PHI_PIN, GPIO_IN, false);
    gpio_init(DBUS_OUT_CE_PIN, GPIO_OUT, true);
    gpio_init(DBUS_IN_CE_PIN, GPIO_OUT, false);
    gpio_init(LED_BUILTIN, GPIO_OUT, true);
}



// Program ROM functions //

bool sram_present = false;
uint8_t sram[SRAM_SIZE];
uint8_t read_program_byte(uint16_t address) {
    if (SRAM_START_ADDR <= address && address < (SRAM_START_ADDR + SRAM_SIZE) && sram_present) {
        return sram[address - SRAM_START_ADDR];
    } else {
        return program_rom[address - PROGRAM_START_ADDR];
    }
}
void write_program_byte(uint16_t address, uint8_t data) {
    if (SRAM_START_ADDR <= address && address < (SRAM_START_ADDR + SRAM_SIZE)) {
        sram_present = true;
        sram[address - SRAM_START_ADDR] = data;
    }
}



// Core 1 loop //

uint8_t tick = 0; // Clock ticks since last WRITE falling edge
uint8_t romc = 0x1C; // IDLE
uint8_t dbus = 0x00;
uint16_t pc0 = 0x00;
uint16_t pc1 = 0x00;
uint16_t dc0 = 0x00;
uint16_t dc1 = 0x00;
uint8_t phiState;
uint8_t lastPhiState = false;
uint8_t writeState;
uint8_t lastWriteState = false;
uint16_t tmp;
bool out_op = false;

inline void execute_romc() __attribute__((always_inline));
inline void execute_romc() {
    switch (romc) {
        case 0x00:
            /*      
             * Instruction Fetch. The device whose address space includes the
             * contents of the PC0 register must place on the data bus the op
             * code addressed by PC0; then all devices increment the contents
             * of PC0.
             */
            dbus = read_program_byte(pc0);
            out_op = pc0 >= PROGRAM_START_ADDR;
            pc0 += 1;
            break;
        case 0x01:
            /*
             * The device whose address space includes the contents of the PC0
             * register must place on the data bus the contents of the memory
             * location addressed by PC0; then all devices add the 8-bit value
             * on the data bus as signed binary number to PC0.
             */
            if (pc0 >= PROGRAM_START_ADDR) {
                dbus = read_program_byte(pc0);
                out_op = true;
            }
            pc0 += dbus;
            break;
        case 0x02:
            /*
             * The device whose DC0 addresses a memory word within the address
             * space of that device must place on the data bus the contents of
             * the memory location addressed by DC0; then all devices increment
             * DC0.
             */
            dbus = read_program_byte(dc0);
            out_op = dc0 >= PROGRAM_START_ADDR;
            dc0 += 1;
            break;
        case 0x03:
            /*
             * Similiar to 0x00, except that it is used for immediate operands
             * fetches (using PC0) instead of instruction fetches.
             */
            dbus = read_program_byte(pc0);
            out_op = pc0 >= PROGRAM_START_ADDR;
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
            dbus = dc0 >> 8;
            out_op = dc0 >= PROGRAM_START_ADDR;
            break;
        case 0x07:
            /*
             * Place the high order byte of PC1 on the data bus.
             * 
             * Note: Assumed to only apply to the device whose address space 
             * includes the contents of the PC1 register
             */
            dbus = pc1 >> 8;
            out_op = pc1 >= PROGRAM_START_ADDR;
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
            dbus = dc0 & 0xff;
            out_op = dc0 >= PROGRAM_START_ADDR;
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
            dbus = pc1 & 0xff;
            out_op = pc1 >= PROGRAM_START_ADDR;
            break;
        case 0x0C:
            /*
             * The device whose address space includes the contents of the PC0
             * register must place the contents of the memory word addressed
             * by PC0 into the data bus; then all devices move the value that
             * has just been placed on the data bus into the low order byte of PC0.
             */
            if (pc0 >= PROGRAM_START_ADDR) {
                dbus = read_program_byte(pc0);
                out_op = true;
            }
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
            if (pc0 >= PROGRAM_START_ADDR) {
                dbus = read_program_byte(pc0);
                out_op = true;
            }
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
            break;
        case 0x10:
            /*
             * Inhibit any modification to the interrupt priority logic.
             */
            // TODO
            break;
        case 0x11:
            /*
             * The device whose address space includes the contents of PC0
             * must place the contents of the addressed memory word on the
             * data bus. All devices must then move the contents of the
             * data bus to the upper byte of DC0.
             */
            if (pc0 >= PROGRAM_START_ADDR) {
                dbus = read_program_byte(pc0);
                out_op = true;
            }
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
            // TODO
            break;
        case 0x1B:
            /*
             * During the prior cycle, the data bus specified the address of an
             * I/O port. The device containing the addressed I/O port must place
             * the contents of the I/O port on the data bus. (Note that the
             * contents of timer and interrupt control registers cannot be read
             * back onto the data bus).
             */
            // TODO
            break;
        case 0x1C:
            /*
             * None.
             *
             * Note: It's function is listed as IDLE in the Fairchild F3850 CPU
             * datasheet. Used in the RESET and INTRPT intructions.
             */
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
            dbus = pc0 & 0xff;
            out_op = pc0 >= PROGRAM_START_ADDR;
            break;
        case 0x1F:
            /*
             * The devices whose address space includes the contents of PC0
             * must place the high order byte of PC0 onto the data bus.
             */
            dbus = (pc0 >> 8) & 0xff;
            out_op = pc0 >= PROGRAM_START_ADDR;
            break;
      }
}

bool fifo_write_successful;
void loop1() {

    writeState = gpio_get(WRITE_PIN);
    if (writeState != lastWriteState) {
        if (writeState == false) { // Falling edge
            
            if (sem_available(&debugCoreSemaphore) == 0) {
                rp2040.fifo.push_nb(
                    (pc0 << 16) |
                    pc1
                );
                fifo_write_successful = rp2040.fifo.push_nb(
                    (phiState << 31) |
                    (writeState << 30) |
                    (outputInstruction << 29) |
                    (romc << 24) |
                    (dbus << 16) |
                    dc0
                );
                if (!fifo_write_successful) { // FIFO full
                    sem_release(&debugCoreSemaphore); // sem = 1
                }
            }
            
            tick = 0;
            gpio_put(DBUS_OUT_CE_PIN, true);            // Disable output buffer
            gpio_set_dir_in_masked(0xFF << DBUS0_PIN);  // Set DBUS to input mode
            gpio_put(DBUS_IN_CE_PIN, false);            // Enable input buffer
        } else {  // Rising edge

            dbus = read_dbus();
            romc = read_romc();
            execute_romc();

            if (out_op) { // Rising edge 
                out_op = false;
                gpio_put(DBUS_IN_CE_PIN, true);              // Disable input buffer
                gpio_clr_mask(0xFF << DBUS0_PIN);            // Write to DBUS
                gpio_set_mask(dbus << DBUS0_PIN);
                gpio_set_dir_out_masked(0xFF << DBUS0_PIN);  // Set DBUS to output mode
                gpio_put(DBUS_OUT_CE_PIN, false);            // Enable output buffer
            }
        }
    }
    lastWriteState = writeState;

    phiState = gpio_get(PHI_PIN);
    if ((phiState != lastPhiState) && phiState) { // Rising edge
        switch (tick) {
            case 0:
            case 1:  // ROMC is now valid
            case 2:
            case 3:
            case 4:  // DBUS is now valid
            case 5:  // WRITE is high
            ;
        }
        tick += 1;
    }
    lastPhiState = phiState;
}

// Running on core 0
void setup() {
    sleep_ms(500);
    Serial.begin(500000);
}

__attribute__((always_inline)) inline void output_debug_info() {
    rp2040.fifo.pop_nb(&queue_H); // (pc0 << 16) | (pc1)
    rp2040.fifo.pop_nb(&queue_L); // (PHI << 31) | (WRITE << 30) | (outOp << 29) | (romc << 24) | (dbus << 16) | (dc0)

    Serial.print((queue_L >> 24) & 0x1F, HEX); // ROMC, 5-bit
    Serial.print(" ");
    Serial.print((queue_L >> 16) & 0xFF, HEX); // DBUS, 8-bit
    Serial.print(" ");
    Serial.print(queue_H >> 16, HEX); // PC0, 16-bit
    Serial.print(" ");
    Serial.print(queue_H & 0xFFFF, HEX); // PC1, 16-bit
    Serial.print(" ");
    Serial.print(queue_L & 0xFFFF, HEX); // DC0, 16-bit
    Serial.print(" ");
    Serial.print((queue_L >> 29) & 0x1, HEX); // outOP, 1-bit
    Serial.print(" ");
    Serial.print((queue_L >> 30) & 0x1, HEX); // WRITE, 1-bit
    Serial.print(" ");
    Serial.println((queue_L >> 31) & 0x1, HEX); // PHI, 1-bit
}

void loop() {
    // Quick debugging on core 0
    if (sem_available(&debugCoreSemaphore) == 1) {
        gpio_put(LED_BUILTIN, true);
        output_debug_info(); // romc, dbus, pc0, pc1, dc0, outOp, WRITE, PHI
        output_debug_info();
        output_debug_info();
        output_debug_info();
        Serial.write(0xA);
        sem_acquire_blocking(&debugCoreSemaphore); // sem = 0
    } else {
        gpio_put(LED_BUILTIN, false);
    }
}
