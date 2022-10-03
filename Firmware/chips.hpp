class ChipType {
    public:
        virtual uint8_t read(uint16_t address) = 0;
        virtual void write(uint16_t address, uint8_t data) = 0;
        virtual bool has_data() = 0;
};
class ROM_CT : public ChipType {
    public:
        ROM_CT() = default;
        uint8_t read(uint16_t address) {
            return program_rom[address];
        }
        void write(uint16_t address, uint8_t data) {}
        bool has_data() {
            return true;
        }
};
class RAM_CT : public ChipType {
    public:
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
class LED_CT : public ChipType {
    public:
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
class NVRAM_CT : public ChipType {
    public:
        NVRAM_CT() = default;
        uint8_t read(uint16_t address) { // TODO: implement NVRAM read
            return -1;
        }
        void write(uint16_t address, uint8_t data) {} // TODO: implement NVRAM write
        bool has_data() {
            return true;
        }
};
ChipType* ChipTypes[] = {new ROM_CT(), new RAM_CT(), new LED_CT(), new NVRAM_CT()};
