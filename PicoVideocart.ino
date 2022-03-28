#include "Videocart12.h"

constexpr uint8_t WRITE_PIN = 17;
constexpr uint8_t PHI_PIN = 26;
constexpr uint8_t DBUS_PINS[8] = {6, 7, 8, 9, 10, 11, 12, 13}; //D0 - D7; Slow but useful for debugging
constexpr uint8_t ROMC_PINS[5] = {18, 19, 20, 21, 22}; //ROMC0 - ROMC4; Slow but useful for debugging

constexpr uint8_t DBUS_IN_CE_PIN = 15;
constexpr uint8_t DBUS_OUT_CE_PIN = 14;

uint32_t* SIO_BASE = 0xd0000000;

void setup() {
    // Initialize cartridge pins
    for (uint8_t i = 0; i < 8; i++) {
        pinMode(DBUS_PINS[i], INPUT_PULLUP); // Default to high impedance
    }
    
    // Default to input mode on dbus
    digitalWrite(DBUS_OUT_CE_PIN, HIGH);
    pinMode(DBUS_OUT_CE_PIN, OUTPUT);
    
    digitalWrite(DBUS_IN_CE_PIN, LOW);
    pinMode(DBUS_IN_CE_PIN, OUTPUT);

    // Use the LED for simple debugging
    pinMode(LED_BUILTIN, OUTPUT);

}

class Program {
    public:
        Program();
        uint8_t read_byte(uint16_t address);
        void write_byte(uint16_t address, uint8_t data);
    private:
        uint8_t program_data[1] = {0}; // 62K max
        bool sram_present = false;
        uint8_t sram[0x800] = {0}; // 2K SRAM
};
Program::Program() {}
uint8_t Program::read_byte(uint16_t address) {
    if (0x2800 <= address && address < 0x3000 && this->sram_present) {
        return this->sram[address - 0x2800];
    } else {
        //return this->program_data[address];
        return program_rom[address - 0x0800];
    }
}
void Program::write_byte(uint16_t address, uint8_t data) {
    if (0x2800 <= address && address < 0x3000) {
        if (!this->sram_present) {
            this->sram_present = true;
        }
        this->sram[address - 0x2800] = data;
    }
    
}
Program program;
//program_rom

uint8_t getRomc(void) {
    //TODO: read directly from registers
    // romc_data = (SIO_BASE[1] >> 18) & 0x1F
    uint8_t romc_data = 0;
    for (uint8_t i = 0; i < 5; i++) {
        bitWrite(romc_data, i, digitalRead(ROMC_PINS[i]));
    }
    return romc_data;
}
uint8_t getDbus(void) {
    //TODO: read directly from registers
    // dbus_data = (SIO_BASE[1] >> 6)
    uint8_t dbus_data;
    for (uint8_t i = 0; i < 8; i++) {
        bitWrite(dbus_data, i, digitalRead(DBUS_PINS[i]));
    }
    return dbus_data;
}

uint8_t tick = 0; // Clock ticks since last WRITE falling edge
uint8_t romc = 0x1C; // IDLE
uint8_t dbus = 0x00;
uint16_t pc0 = 0x00;
uint16_t pc1 = 0x00;
uint16_t dc0 = 0x00;
uint16_t dc1 = 0x00;
uint8_t phiState;
uint8_t lastPhiState = LOW;
uint8_t writeState;
uint8_t lastWriteState = LOW;
uint16_t tmp;
void loop() {

    // writeState = (SIO_BASE[1] >> WRITE_PIN) & 1
    writeState = digitalRead(WRITE_PIN);
    if (writeState != lastWriteState) {
        if (writeState == LOW) { // Falling edge
            tick = 0;
            
            // SIO_BASE[5] = 1 << DBUS_OUT_CE_PIN   // Set DBUS_OUT_CE_PIN
            digitalWrite(DBUS_OUT_CE_PIN, HIGH);
            
            // SIO_BASE[10] = 0xFF << 6     // Place DBUS in input mode
            for (uint8_t i = 0; i < 8; i++) {
                digitalWrite(DBUS_PINS[i], LOW);
                pinMode(DBUS_PINS[i], INPUT_PULLUP);
            }
            
            // SIO_BASE[6] = 1 << DBUS_IN_CE_PIN   // Clear DBUS_IN_CE_PIN
            digitalWrite(DBUS_IN_CE_PIN, LOW);
            
        } else { // Rising edge
            
            // SIO_BASE[5] = 1 << DBUS_IN_CE_PIN   // Set DBUS_IN_CE_PIN
            digitalWrite(DBUS_IN_CE_PIN, HIGH);

            // SIO_BASE[9] = 0xFF << 6     // Place DBUS in output mode
            // SIO_BASE[5] = dbus << 6     // Set DBUS high bits
            // SIO_BASE[6] = (!dbus) << 6  // Clear DBUS low bits
            for (uint8_t i = 0; i < 8; i++) {
                pinMode(DBUS_PINS[i], OUTPUT);
                digitalWrite(DBUS_PINS[i], bitRead(dbus, i));
            }
            
            // SIO_BASE[6] = 1 << DBUS_OUT_CE_PIN   // Clear DBUS_OUT_CE_PIN
            digitalWrite(DBUS_OUT_CE_PIN, LOW);
        }
    }
    lastWriteState = writeState;

    // phiState = (SIO_BASE[1] >> PHI_PIN) & 1
    phiState = digitalRead(PHI_PIN);
    if (phiState != lastPhiState && phiState == HIGH) { // Rising edge
  
        if (tick == 1) { // Wait for the second tick for ROMC to stabalize
            dbus = getDbus(); // is this valid at tick 1, or only when write is high?
            switch (getRomc()) {
              case 0x00:
                  /*      
                   * Instruction Fetch. The device whose address space includes the
                   * contents of the PC0 register must place on the data bus the op
                   * code addressed by PC0; then all devices increment the contents
                   * of PC0.
                   */
                  dbus = program.read_byte(pc0);
                  pc0 += 1;
                  break;
              case 0x01:
                  /*
                   * The device whose address space includes the contents of the PC0
                   * register must place on the data bus the contents of the memory
                   * location addressed by PC0; then all devices add the 8-bit value
                   * on the data bus as signed binary number to PC0.
                   */
                  dbus = program.read_byte(pc0);
                  pc0 += dbus;
                  break;
              case 0x02:
                  /*
                   * The device whose DC0 addresses a memory word within the address
                   * space of that device must place on the data bus the contents of
                   * the memory location addressed by DC0; then all devices increment
                   * DC0.
                   */
                  dbus = program.read_byte(dc0);
                  dc0 += 1;
                  break;
              case 0x03:
                  /*
                   * Similiar to 0x00, except that it is used for immediate operands
                   * fetches (using PC0) instead of instruction fetches.
                   */
                  dbus = program.read_byte(pc0);
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
                   */
                  program.write_byte(dc0, dbus);
                  dc0 += 1;
                  break;
              case 0x06:
                  /*
                   * Place the high order byte of DC0 on the data bus.
                   */
                  dbus = dc0 >> 8;
                  break;
              case 0x07:
                  /*
                   * Place the high order byte of PC1 on the data bus.
                   */
                  dbus = pc1 >> 8;
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
                  break;
              case 0x0C:
                  /*
                   * The device whose address space includes the contents of the PC0
                   * register must place the contents of the memory word addressed
                   * by PC0 into the data bus; then all devices move the value that
                   * has just been placed on the data bus into the low order byte of PC0.
                   */
                  dbus = program.read_byte(pc0);
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
                  dbus = program.read_byte(pc0);
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
                  dbus = program.read_byte(pc0);
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
                  break;
              case 0x1F:
                  /*
                   * The devices whose address space includes the contents of PC0
                   * must place the high order byte of PC0 onto the data bus.
                   */
                  dbus = (pc0 >> 8) & 0xff;
                  break;
            }
        }
        tick += 1;
    }
    lastPhiState = phiState;
}
