/** \file ports.h
 * 
 * Channel F I/O Ports
 *
 * The Channel F has 256 addressable I/O ports that it communicates with via the
 * OUT and IN instructions. The Channel F has 4 of these ports assigned to the
 * CPU/PSU. More information on the CPU/PSU ports can be found at
 * https://channelf.se/veswiki/index.php?title=Port
 * 
 * Default Port Assignments:
 * 
 *  Port Address   | Device           | Description
 *  ---------------|------------------|-------------
 *  0              | CPU              | buttons and Video RAM
 *  1              | CPU              | right controller and Video palette
 *  4              | PSU              | left controller and horizontal video position
 *  5              | PSU              | sound and vertical video position
 *  12             | 3853 SMI         | programmable interrupt vector (upper byte) 
 *  13             | 3853 SMI         | programmable interrupt vector (lower byte) 
 *  14             | 3853 SMI         | interrupt control port
 *  15             | 3853 SMI         | programmable timer
 *  24             | Videocart 10/18  | 2102 SRAM
 *  25             | Videocart 10/18  | 2102 SRAM
 * 
 */

// Abstract base class
class IOPort {
    public: 
        virtual uint8_t read() = 0;
        virtual void write(uint8_t) = 0;
};

/**
 * Implementation of a 2102 SRAM IC
 *
 * Only used in Videocart 10 (Maze) and 18 (Hangman)
 * More info found at: http://seanriddle.com/mazepat.asm
 * Port descriptions:
 *                   7   6   5   4   3   2   1   0
 *     port a (p24) OUT  -   -   -  IN  A2  A3  RW
 *     port b (p25) A9  A8  A7  A1  A6  A5  A4  A0
 *
 */
class Sram2102 : public IOPort {
    private: 
        bool sram_data[1024];  // 1K x 1 bits
        uint16_t address;      // 10-bit address
        uint8_t port_a; 
        uint8_t port_b;
        uint8_t port_index;
    public: 
        Sram2102(uint8_t port_index): port_index(port_index) {}
    
        uint8_t read() {
            return port_index ? port_b : port_a;
        }
    
        void write(uint8_t data) {
            if (port_index) {
                port_b = data;
            } else {
                port_a = data & 0xF;
            }
            address = (port_a & 0x6) << 7 | port_b;
            if (port_a & 1) { // RW == write
                sram_data[address] = port_a & 0x8;
            }
            port_a = sram_data[address] << 7 | port_a & 0x7F;
        }
};
