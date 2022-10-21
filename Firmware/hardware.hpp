/** \file hardware.hpp
 * 
 * \brief The hardware type determines which ports (if any) are available
*/

// FIXME: use global IOPorts

class HardwareType {
    public:
        virtual void initialize_ports(IOPort* IOPorts[256]) = 0;
};

class VIDEOCART_HT : public HardwareType {
    public:
        static constexpr uint16_t id = 0;
        VIDEOCART_HT() = default;
        void initialize_ports(IOPort* IOPorts[256]) {}
};

class VIDEOCART_10_18_HT : public HardwareType {
    public:
        static constexpr uint16_t id = 1;
        VIDEOCART_10_18_HT() = default;
        void initialize_ports(IOPort* IOPorts[256]) {
            IOPorts[0x20] = new Sram2102(0);
            IOPorts[0x21] = new Sram2102(1);
            IOPorts[0x24] = new Sram2102(0);
            IOPorts[0x25] = new Sram2102(1);
        }
};

class ROM_RAM_HT : public HardwareType {
    public:
        static constexpr uint16_t id = 2;
        ROM_RAM_HT() = default;
        void initialize_ports(IOPort* IOPorts[256]) {}
};

class SABA_HT : public HardwareType {
    public:
        static constexpr uint16_t id = 3;
        SABA_HT() = default;
        void initialize_ports(IOPort* IOPorts[256]) {}
};

class MULTICART_HT : public HardwareType {
    public:
        static constexpr uint16_t id = 4;
        MULTICART_HT() = default;
        void initialize_ports(IOPort* IOPorts[256]) {}
};

class FLASHCART_HT : public HardwareType {
    public:
        static constexpr uint16_t id = 5;
        FLASHCART_HT() = default;
        void initialize_ports(IOPort* IOPorts[256]) {
            IOPorts[0x20] = new Sram2102(0);
            IOPorts[0x21] = new Sram2102(1);
            IOPorts[0x24] = new Sram2102(0);
            IOPorts[0x25] = new Sram2102(1);
            IOPorts[0xFF] = new Launcher(file_data);
        }
};

inline HardwareType* HardwareTypes[] = {
    new VIDEOCART_HT(),
    new VIDEOCART_10_18_HT(),
    new ROM_RAM_HT(),
    new SABA_HT(),
    new MULTICART_HT(),
    new FLASHCART_HT()
};