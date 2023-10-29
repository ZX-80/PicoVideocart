/** \file gpio.hpp 
 * 
 * \brief General Purpose Input/Output (GPIO) functionality for the Teensy 4.0
 *
 * \details The Teensy 4.0 has 40 multi-functional General Purpose Input / Output (GPIO) pins. These pins
 * default to INPUT with a "keeper" resistor. Teensy 4.0 pins accept 0 to 3.3V signals. The pins are not 5V 
 * tolerant. Various features are provided, such as Adjustable Output Bandwidth, Variable Drive Strength, 
 * Slew Rate Limiting, Pulse Width Modulation (31 pins), Analog Inputs (14 pins), and Pin Change Interrupts.
 * An LED is provided on pin 13.
 * 
 * Inputs may use keeper, pullup, pulldown resistors, or be disabled (minimizes power consumption).
 * Outputs can be open drain.
 * 
 * ## Ports
 * GPIO pins can be directly accessed through their corresponding ports. Four ports exist, and can be quickly
 * accessed via IMXRT_GPIO6 to IMXRT_GPIO9 in imxrt.h. This also allows for modifying multiple gpio pins in
 * one operation.

 * Refer to the i.MX RT1060 Processor Reference Manual & the following link for more information on GPIO.
 * https://www.pjrc.com/store/ic_mkl02_t4.html
 * 
 * ### Pin Assignments
 * 
 * ```
 *  Native     Ports  Function     GPIO                        GPIO    Function            Ports  Native
 *                                         _____|----|_____
 *                                  GND - |      USB       | - VIN
 *  AD_B0_03   1.3        SD_CS     GP0 - |      USB       | - VUSB/GND
 *  AD_B0_02   1.2      FRAM_CS     GP1 - |                | - 3.3V
 *    EMC_04   4.4    SD Detect     GP2 - |                | - GP23    DBUS 5              1.25   AD_B1_09
 *    EMC_05   4.5     SD Write     GP3 - |                | - GP22    DBUS 4              1.24   AD_B1_08
 *    EMC_06   4.6          PHI     GP4 - |                | - GP21    DBUS 7              1.27   AD_B1_11
 *    EMC_08   4.8           RW     GP5 - |                | - GP20    DBUS 6              1.26   AD_B1_10
 *     B0_10   2.10       ROMC4     GP6 - |                | - GP19    DBUS Direction      1.16   AD_B1_00
 *     B1_01   2.17       ROMC3     GP7 - |                | - GP18    Interrupt Request   1.17   AD_B1_01
 *     B1_00   2.16       ROMC2     GP8 - |   Raspberry    | - GP17    DBUS 2              1.22   AD_B1_06
 *     B0_11   2.11       ROMC1     GP9 - |       Pi       | - GP16    DBUS 3              1.23   AD_B1_07
 *     B0_00   2.0        ROMC0    GP10 - |      Pico      | - GP15    DBUS 1              1.19   AD_B1_03
 *     B0_02   2.2         MOSI    GP11 - |                | - GP14    DBUS 0              1.18   AD_B1_02
 *     B0_01   2.1         MISO    GP12 - |_--_--_--_--_--_| - GP13    SCK/LED             2.3    B0_03
 *                                 VBUS ____/  /   |  \  \____ On/Off
 *                                 3.3V ______/   GND  \______ Program
 * 
 *                                          BOTTOM SMT PINS
 *                                        ------------------                                          
 *  AD_B0_12   1.12  Bus Presence  GP24 - |  =====  =====  | - GP25    Board LED           1.13   AD_B0_13
 *  AD_B1_14   1.30           X    GP26 - |  =====  =====  | - GP27    X                   1.31   AD_B1_15
 *    EMC_32   3.18           X    GP28 - |  =====  =====  | - GP29    X                   4.31   EMC_31
 *    EMC_37   3.23           X    GP30 - |  =====  =====  | - GP31    X                   3.22   EMC_36
 *     B0_12   2.12           X    GP32 - |  =====  =====  | - GP33    X                   4.7    EMC_07
 *                                        ------------------
 * ```
 */

#pragma once

// GPIO pin mappings may seem arbitrary, but that's because we're accessing
// ports (to allow reading multiple pins at once), which have different mappings

// Data bus (Port 1: xxxx765432xx10xxxxxxxxxxxxxxxxxx)
extern uint8_t dbus;
inline constexpr uint32_t DBUS_PINS_BITMASK = 0xFCC0000;
inline constexpr uint8_t DBUS_MODE_PIN = 19; // 0 = INPUT, 1 = OUTPUT
inline constexpr uint8_t DBUS0_PIN = 14;
inline constexpr uint8_t DBUS1_PIN = 15;
inline constexpr uint8_t DBUS2_PIN = 17;
inline constexpr uint8_t DBUS3_PIN = 16;
inline constexpr uint8_t DBUS4_PIN = 22;
inline constexpr uint8_t DBUS5_PIN = 23;
inline constexpr uint8_t DBUS6_PIN = 20;
inline constexpr uint8_t DBUS7_PIN = 21;

// Command bus (Port 2: xxxxxxxxxxxxxx12xxxx30xxxxxxxxx4)
inline constexpr uint32_t ROMC_PINS_BITMASK = 0x30C01;
inline constexpr uint8_t ROMC0_PIN = 10;
inline constexpr uint8_t ROMC1_PIN = 6;
inline constexpr uint8_t ROMC2_PIN = 9;
inline constexpr uint8_t ROMC3_PIN = 8;
inline constexpr uint8_t ROMC4_PIN = 7;
inline constexpr uint8_t WRITE_PIN = 5;
inline constexpr uint8_t PHI_PIN = 4;

// Other Pins
inline constexpr uint8_t LED_BOARD = 25;
inline constexpr uint8_t CHANNEL_F_BUS_PRESENCE = 24; // Active low
inline constexpr uint8_t INTRQ_PIN = 18;              // Active low


/*!
 * \brief Set the gpio pins in mask to input mode
 *
 * \param gpio The gpio port to modify
 * \param mask The pins to be modified
 */
__always_inline void gpio_set_dir_in_masked(IMXRT_GPIO_t gpio, uint32_t mask) {
  gpio.GDIR &= ~mask; //clear (set pins to input)
}

/*!
 * \brief Set the gpio pins in mask to output mode
 *
 * \param gpio The gpio port to modify
 * \param mask The pins to be modified
 */
__always_inline void gpio_set_dir_out_masked(IMXRT_GPIO_t gpio, uint32_t mask) {
  gpio.GDIR |= mask; //set (set pins to output)
}

/*! \brief Set the direction of the data bus drivers
 *
 * \param mode INPUT or OUTPUT
 */
__always_inline void dbus_mode(uint8_t mode) {
    switch (mode) {
        case OUTPUT:
            digitalWriteFast(DBUS_MODE_PIN, HIGH);
            gpio_set_dir_out_masked(IMXRT_GPIO6, DBUS_PINS_BITMASK);
        case INPUT:
            gpio_set_dir_in_masked(IMXRT_GPIO6, DBUS_PINS_BITMASK);
            digitalWriteFast(DBUS_MODE_PIN, LOW);
    }
}


/*! \brief Get the data bus value
 *
 * \return 8-bit data bus value
 */
__always_inline uint8_t read_dbus() { 
    uint32_t data = GPIO6_PSR >> 18;                // data = 000000000000000000xxxx765432xx10| load and shift 18 bits  | 2 cycles
    asm volatile("bfi %0, %0, 2, 2" : "+r"(data));  // data = 000000000000000000xxxx7654321010| insert bits 0-1 into 2-3| 1 cycle
    return data >> 2;                               // data = 76543210                        | extract byte            | 1 cycle
}

/*! \brief Put a value on the data bus
 * 
 * \param value The byte to write
 */
__always_inline void write_dbus(uint8_t value) {
    dbus = value;
    asm volatile("bfi %0, %1, 18, 2" : "+r"(GPIO6_DR) : "r"(dbus));      // GPIO6 = xxxxxxxxxxxx10xxxxxxxxxxxxxxxxxx| 1 cycle
    asm volatile("bfi %0, %1, 22, 6" : "+r"(GPIO6_DR) : "r"(dbus >> 2)); // GPIO6 = xxxx765432xx10xxxxxxxxxxxxxxxxxx| 1 cycle
    dbus_mode(OUTPUT);                                                   // Enable output buffer                    | 2 cycles
}

/*! \brief Put a value on the data bus
 * 
 * \param value The byte to write
 * \param addr_source The address being targeted
 */
__always_inline void write_dbus(uint8_t value, uint16_t addr_source) {
    if (program_attribute[addr_source] != RESERVED_CT::id) {
        write_dbus(value);
    }
}

/*! \brief Map ROMC bits to their pins
 *
 * \param romc_command 5-bit romc command
 * \return 8-bit ROMC bus value
 */
consteval uint8_t ROMC_PIN_MAP(uint8_t romc_command) { 
    // 43210 -> 12LLL430
    return bitRead(romc_command, 0)        |
           (bitRead(romc_command, 1) << 7) |
           (bitRead(romc_command, 2) << 6) |
           (bitRead(romc_command, 3) << 1) |
           (bitRead(romc_command, 4) << 2);
}

/*! \brief Get the ROMC bus value
 *
 * \return 5-bit ROMC bus value
 */
__always_inline uint8_t read_romc() {
    // Sparse values prevent the generation of jump tables. Use port 4.4-4.8 in future PCBs to save a few cycles

    uint32_t romc = GPIO7_PSR;                       // data = xxxxxxxxxxxxxx12xxxx30xxxxxxxxx4| load                 | 1 cycle
    asm volatile("bfi %0, %0, 12, 1" : "+r"(romc));  // data = xxxxxxxxxxxxxx12xxx430xxxxxxxxx4| insert bit 0 into 12 | 1 cycle
    return (romc >> 10) & 0b11000111;                // data = LLLLLLLLLLLLLL12LLL430LLLLLLLLLL| clear unused bits    | 1 cycle
                                                     // data = 12LLL430                        | extract byte         | 1 cycle

    // This version cost 1 cycle less, but gives up GPIO 32
    // Requires GPIO_B0_15 / GPIO_B0_14 / GPIO_B0_12 read zero
    // uint32_t romc = GPIO7_PSR;                       // data = xxxxxxxxxxxxxx12LLxL30xxxxxxxxx4| load                 | 1 cycle
    // asm volatile("bfi %0, %0, 13, 1" : "+r"(romc));  // data = xxxxxxxxxxxxxx12LL4L30xxxxxxxxx4| insert bit 0 into 13 | 1 cycle
    // asm volatile("ubfx %0, %0, 10, 8" : "+r"(romc)); // data = 12LL4L30                        | extract bits 2-10    | 1 cycle 
}

/*!
 * \brief Set INTRQ high
 */
__always_inline void trigger_interrupt_request() {
    digitalWriteFast(INTRQ_PIN, HIGH);
}

/*!
 * \brief Set INTRQ low
 */
__always_inline void reset_interrupt_request() {
    digitalWriteFast(INTRQ_PIN, LOW);
}