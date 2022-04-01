#include <hardware/structs/iobank0.h>
#include "Videocart12.h"

constexpr uint8_t WRITE_PIN = 17;
constexpr uint8_t PHI_PIN = 26;
constexpr uint8_t DBUS0_PIN = 6;
constexpr uint8_t ROMC0_PIN = 18;

constexpr uint8_t DBUS_IN_CE_PIN = 15;
constexpr uint8_t DBUS_OUT_CE_PIN = 14;



// GPIO functions //

inline void gpio_init(uint8_t pin, bool mode) __attribute__((always_inline));
inline void gpio_init(uint8_t pin, bool mode) {
    iobank0_hw -> io[pin].ctrl &= 0xFFE0;
    iobank0_hw -> io[pin].ctrl |= ((2 + mode) << 12) + GPIO_FUNC_SIO;
}

inline void gpio_set(uint8_t pin) __attribute__((always_inline));
inline void gpio_set(uint8_t pin) {
    sio_hw->gpio_set = 1 << pin;
}

inline void gpio_clear(uint8_t pin) __attribute__((always_inline));
inline void gpio_clear(uint8_t pin) {
    sio_hw->gpio_clr = 1 << pin;
}

inline bool gpio_in(uint8_t pin) __attribute__((always_inline));
inline bool gpio_in(uint8_t pin) {
    return (sio_hw->gpio_in >> pin) & 1;
}

inline uint8_t read_romc() __attribute__((always_inline));
inline uint8_t read_romc() {
    return (sio_hw->gpio_in >> ROMC0_PIN) & 0x1F;
}

inline uint8_t read_dbus() __attribute__((always_inline));
inline uint8_t read_dbus() {
    return (sio_hw->gpio_in >> DBUS0_PIN) & 0xFF;
}

inline void write_dbus(uint8_t value) __attribute__((always_inline));
inline void write_dbus(uint8_t value) {   
    sio_hw->gpio_set = value << DBUS0_PIN;
    sio_hw->gpio_clr = (!value) << DBUS0_PIN;
}

inline void dbus_input_mode() __attribute__((always_inline));
inline void dbus_input_mode() {
    sio_hw->gpio_oe_clr = 0xFF << DBUS0_PIN;
}

inline void dbus_output_mode() __attribute__((always_inline));
inline void dbus_output_mode() {
    sio_hw->gpio_oe_set = 0xFF << DBUS0_PIN;
}

void setup() {
    // Initialize cartridge pins
    for (uint8_t i = 0; i < 8; i++) {
        gpio_init(DBUS0_PIN + i, GPIO_IN);
    }
    for (uint8_t i = 0; i < 5; i++) {
        gpio_init(ROMC0_PIN + i, GPIO_IN);
    }
    
    gpio_init(WRITE_PIN, GPIO_IN);    
    gpio_init(PHI_PIN, GPIO_IN);
    
    gpio_init(DBUS_OUT_CE_PIN, GPIO_OUT);
    gpio_set(DBUS_OUT_CE_PIN);

    gpio_init(DBUS_IN_CE_PIN, GPIO_OUT);
    gpio_clear(DBUS_IN_CE_PIN);

    // Use the LED for simple debugging
    gpio_init(LED_BUILTIN, GPIO_OUT);
    gpio_set(LED_BUILTIN);

}



// ROM/SRAM functions //

bool sram_present = false;
uint8_t sram[0x800] = {0}; // 2K SRAM

inline uint8_t read_program_byte(uint16_t address) __attribute__((always_inline));
inline uint8_t read_program_byte(uint16_t address) {
    if (0x2800 <= address && address < 0x3000 && sram_present) {
        return sram[address - 0x2800];
    } else {
        return program_rom[address - 0x0800];
    }
}

inline void write_program_byte(uint16_t address, uint8_t data) __attribute__((always_inline));
inline void write_program_byte(uint16_t address, uint8_t data) {
    if (0x2800 <= address && address < 0x3000) {
        if (!sram_present) {
            sram_present = true;
        }
        sram[address - 0x2800] = data;
    }
}



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
void loop() {

    writeState = gpio_in(WRITE_PIN);
    if (writeState != lastWriteState) {
        if (!writeState) { // Falling edge
            tick = 0;
            gpio_set(DBUS_OUT_CE_PIN);  // Disable output buffer
            dbus_input_mode();
            gpio_clear(DBUS_IN_CE_PIN); // Enable input buffer
        } else if (out_op) { // Rising edge
            out_op = false;
            gpio_set(DBUS_IN_CE_PIN);    // Disable input buffer
            dbus_output_mode();   
            write_dbus(dbus);
            gpio_clear(DBUS_OUT_CE_PIN); // Enable output buffer
        }
    }
    lastWriteState = writeState;

    phiState = gpio_in(PHI_PIN);
    if (phiState != lastPhiState && phiState) { // Rising edge
        
        if (tick == 1) { // Wait for the second tick for ROMC to stabilize
            romc = read_romc();
            switch (romc) {
              case 0x00:
                  /*      
                   * Instruction Fetch. The device whose address space includes the
                   * contents of the PC0 register must place on the data bus the op
                   * code addressed by PC0; then all devices increment the contents
                   * of PC0.
                   */
                  dbus = read_program_byte(pc0);
                  pc0 += 1;
                  out_op = true;
                  break;
              case 0x01:
                  /*
                   * The device whose address space includes the contents of the PC0
                   * register must place on the data bus the contents of the memory
                   * location addressed by PC0; then all devices add the 8-bit value
                   * on the data bus as signed binary number to PC0.
                   */
                  dbus = read_program_byte(pc0);
                  pc0 += dbus;
                  out_op = true;
                  break;
              case 0x02:
                  /*
                   * The device whose DC0 addresses a memory word within the address
                   * space of that device must place on the data bus the contents of
                   * the memory location addressed by DC0; then all devices increment
                   * DC0.
                   */
                  dbus = read_program_byte(dc0);
                  dc0 += 1;
                  out_op = true;
                  break;
              case 0x03:
                  /*
                   * Similiar to 0x00, except that it is used for immediate operands
                   * fetches (using PC0) instead of instruction fetches.
                   */
                  dbus = read_program_byte(pc0);
                  pc0 += 1;
                  out_op = true;
                  break;
              case 0x04:
                  /*
                   * Copy the contents of PC1 into PC0
                   */
                  pc0 = pc1;
                  break;
              case 0x06:
                  /*
                   * Place the high order byte of DC0 on the data bus.
                   */
                  dbus = dc0 >> 8;
                  out_op = true;
                  break;
              case 0x07:
                  /*
                   * Place the high order byte of PC1 on the data bus.
                   */
                  dbus = pc1 >> 8;
                  out_op = true;
                  break;
              case 0x09:
                  /*
                   * The device whose address space includes the contents of the DC0
                   * register must place the low order byte of DC0 onto the data bus.
                   */
                  dbus = dc0 & 0xff;
                  out_op = true;
                  break;
              case 0x0B:
                  /*
                   * The device whose address space includes the value in PC1
                   * must place the low order byte of PC1 onto the data bus.
                   */
                  dbus = pc1 & 0xff;
                  out_op = true;
                  break;
              case 0x0C:
                  /*
                   * The device whose address space includes the contents of the PC0
                   * register must place the contents of the memory word addressed
                   * by PC0 into the data bus; then all devices move the value that
                   * has just been placed on the data bus into the low order byte of PC0.
                   */
                  dbus = read_program_byte(pc0);
                  pc0 = (pc0 & 0xff00) | dbus;
                  out_op = true;
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
                  dbus = read_program_byte(pc0);
                  dc0 = (dc0 & 0xff00) | dbus;
                  out_op = true;
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
                  dbus = read_program_byte(pc0);
                  dc0 = (dc0 & 0x00ff) | (dbus << 8);
                  out_op = true;
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
                   * None. (IDLE; used in RESET and INTRPT)
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
                  out_op = true;
                  break;
              case 0x1F:
                  /*
                   * The devices whose address space includes the contents of PC0
                   * must place the high order byte of PC0 onto the data bus.
                   */
                  dbus = (pc0 >> 8) & 0xff;
                  out_op = true;
                  break;
            }
        } else if (writeState) {
            dbus = read_dbus(); // Only valid after 4 ticks
            switch (romc) {
                case 0x05:
                    /*
                     * Store the data bus contents into the memory location pointed
                     * to by DC0; increment DC0.
                     */
                    write_program_byte(dc0, dbus);
                    dc0 += 1;
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
                case 0x0A:
                    /*
                     * All devices add the 8-bit value on the data bus, treated as
                     * signed binary number, to the data counter.
                     */
                    dc0 += (int8_t) dbus;
                    break;  
                case 0x12:
                    /*
                     * All devices copy the contents of PC0 into PC1. All devices then
                     * move the contents of the data bus into the low order byte of PC0.
                     */
                    pc1 = pc0;
                    pc0 = (pc0 & 0xff00) | dbus;
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
            }
        }
        tick += 1;
    }
    lastPhiState = phiState;
}
