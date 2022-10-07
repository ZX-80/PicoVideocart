/** \file chips.hpp
 *  
 * \brief The chip type determines how memory is read/written
*/

#pragma once

#include "no_sd_rom.hpp"

inline uint8_t program_attribute[0x10000]; // Determines chip type for that address

/*! \brief Abstract base class for chip types 
 * 
 * \details This interface is used by the Videocart emulation code to read and
 * write to memory addresses. New chip types can be added by implementing this 
 * interface.
*/
class ChipType {
    public:
        virtual uint8_t read(uint16_t address) = 0;
        virtual void write(uint16_t address, uint8_t data) = 0;
        virtual bool has_data() = 0;
};

/*! \brief Read-only memory */
class ROM_CT : public ChipType {
    public:
        static constexpr uint16_t id = 0;
        ROM_CT() = default;
        uint8_t read(uint16_t address) {
            return program_rom[address];
        }
        void write(uint16_t address, uint8_t data) {}
        bool has_data() {
            return true;
        }
};

/*! \brief Read/Write memory */
class RAM_CT : public ChipType {
    public:
        static constexpr uint16_t id = 1;
        RAM_CT() = default;
        uint8_t read(uint16_t address) {
            return program_rom[address];
        }
        void write(uint16_t address, uint8_t data) {
            program_rom[address] = data;
        }
        bool has_data() {
            return false;
        }
};

/*! \brief Similar to ROM, but toggles the LED when written to */
class LED_CT : public ChipType {
    public:
        static constexpr uint16_t id = 2;
        LED_CT() = default;
        uint8_t read(uint16_t address) {
            return program_rom[address];
        }
        void write(uint16_t address, uint8_t data) {
            gpio_xor_mask(1 << LED_BUILTIN); // Toggle LED
        }
        bool has_data() {
            return true;
        }
};

/*! \brief Non-volatile RAM (data is preserved between power cycles) */
class NVRAM_CT : public ChipType {
    public:
        static constexpr uint16_t id = 3;
        NVRAM_CT() = default;
        uint8_t read(uint16_t address) { // TODO: implement NVRAM read
            return -1;
        }
        void write(uint16_t address, uint8_t data) {} // TODO: implement NVRAM write
        bool has_data() {
            return true;
        }
};

/*! \brief Cannot be read/written to */
class RESERVED_CT : public ChipType {
    public:
        static constexpr uint16_t id = 0xFF;
        RESERVED_CT() = default;
        uint8_t read(uint16_t address) {return -1;}
        void write(uint16_t address, uint8_t data) {}
        bool has_data() {return false;}
};
ChipType* ChipTypes[] = {new ROM_CT(), new RAM_CT(), new LED_CT(), new NVRAM_CT()}; //FIXME: use below format
// ChipType* ChipTypes[256];
// ChipTypes[ROM_CT::id] = new ROM_CT();
// ChipTypes[RAM_CT::id] = new RAM_CT();
// ChipTypes[LED_CT::id] = new LED_CT();
// ChipTypes[NVRAM_CT::id] = new NVRAM_CT();