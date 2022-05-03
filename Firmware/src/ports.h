class IO_port { // Abstract base class
    public: 
        virtual uint8_t read() = 0;      
        virtual void write(uint8_t) = 0;
};

class port_sram : public IO_port {
    private: 
        uint16_t address;      // 10-bit address
        bool sram_data[1024];  // 1K x 1 bits
                          //               7   6   5   4   3   2   1   0
        uint8_t port_a;   // port a (p24) OUT  -   -   -  IN  A2  A3  RW
        uint8_t port_b;   // port b (p25) A9  A8  A7  A1  A6  A5  A4  A0
        uint8_t port_index;
    public: 
        port_sram(uint8_t _port_index) {
            port_index = _port_index;
        };
        uint8_t read() {
            switch (port_index) {
                case 0:
                    return port_a;
                case 1:
                    return port_b;
            }
        }
        void write(uint8_t data) {
            switch (port_index) {
                case 0:
                    port_a = data & 0xF;
                    address = (((port_a & 0x6) << 7) | port_b);
                    if (port_a & 1) { // Write to SRAM
                        sram_data[address] = port_a & 0x8;
                    }
                    port_a = (sram_data[address] << 7) | (port_a & 0x7F);
                case 1:
                    port_b = data;
                    address = (((port_a & 0x6) << 7) | port_b);
                    if (port_a & 1) { // Write to SRAM
                        sram_data[address] = port_a & 0x8;
                    }
                    port_a = (sram_data[address] << 7) | (port_a & 0x7F);
            }
        }
};
