/** \file fram.h
 *  \brief a library to connect an FM25V10 FRAM IC to the Raspberry Pi Pico
 * 
 */

#pragma once

#include <SPI.h>

class FM25V10 {
    public:
        FM25V10(uint8_t chipSelect, uint32_t freq = 5000000, SPIClass* theSPI = &SPI) {
            FramChipSelectPin = chipSelect
            _spiSetting = new SPISettings(freq, MSBFIRST, SPI_MODE0);
            _spi = theSPI
        };

        ~FM25V10() {
            delete _spiSetting;
        }

        bool begin() {
            if (!_spi->begin()) {
                return false;
            }
            return checkDeviceID();
        }

        void writeEnable(bool enable) {
            beginTransactionWithAssertingCS();
            if (enable) {
                _spi->transfer(OPCODE_WREN);
            } else {
                _spi->transfer(OPCODE_WRDI);
            }
            endTransactionWithDeassertingCS();
        }

        void write(uint32_t addr, uint8_t value) {
            beginTransactionWithAssertingCS();
            _spi->transfer(OPCODE_WRITE);
            _spi->transfer(addr >> 16);
            _spi->transfer(addr >> 8);
            _spi->transfer(addr);
            _spi->transfer(value);
            endTransactionWithDeassertingCS();
        }

        void write(uint32_t addr, const uint8_t *values, size_t count) {
            beginTransactionWithAssertingCS();
            _spi->transfer(OPCODE_WRITE);
            _spi->transfer(addr >> 16);
            _spi->transfer(addr >> 8);
            _spi->transfer(addr);
            for (uint8_t i = 0; i < count; i++) {
                _spi->transfer(values[i]);
            }
            endTransactionWithDeassertingCS();
        }

        uint8_t read(uint32_t addr) {
            uint8_t val;
            beginTransactionWithAssertingCS();
            _spi->transfer(OPCODE_READ);
            _spi->transfer(addr >> 16);
            _spi->transfer(addr >> 8);
            _spi->transfer(addr);
            val = _spi->transfer(0xFF);
            endTransactionWithDeassertingCS();

            return val;
        }

        void read(uint32_t addr, uint8_t *values, size_t count) {
            beginTransactionWithAssertingCS();
            _spi->transfer(OPCODE_READ);
            _spi->transfer(addr >> 16);
            _spi->transfer(addr >> 8);
            _spi->transfer(addr);
            for (uint8_t i = 0; i < count; i++) {
                values[i] = _spi->transfer(0xFF);
            }
            endTransactionWithDeassertingCS();
        }

        uint8_t readStatusRegister() {
            uint8_t val;
            beginTransactionWithAssertingCS();
            _spi->transfer(OPCODE_RDSR);
            val = _spi->transfer(0xFF);
            endTransactionWithDeassertingCS();

            return val;
        }

        void writeStatusRegister(uint8_t value) {
            beginTransactionWithAssertingCS();
            _spi->transfer(OPCODE_WRSR);
            _spi->transfer(value);
            endTransactionWithDeassertingCS();
        }
        
        void readDeviceID(uint8_t* deviceID) { // MMMMMMMPP
            beginTransactionWithAssertingCS();
            _spi->transfer(OPCODE_RDID);
            for (uint8_t i = 0; i < 9; i++) {
                deviceID[i] = _spi->transfer(0xFF);
            }
            endTransactionWithDeassertingCS();
        }

        void readSerialNumber(uint8_t* serialNumber) { // CI, CI, SN, SN, SN, SN, SN, CRC
            beginTransactionWithAssertingCS();
            _spi->transfer(OPCODE_SNR);
            for (uint8_t i = 0; i < 8; i++) {
                serialNumber[i] = _spi->transfer(0xFF);
            }
            endTransactionWithDeassertingCS();
        }

        void sleep() {
            beginTransactionWithAssertingCS();
            _spi->transfer(OPCODE_SLEEP);
            endTransactionWithDeassertingCS();
        }

    private: 
        SPIClass* _spi;
        SPISettings* _spiSetting;
        uint8_t FramChipSelectPin;

        constexpr uint8_t OPCODE_WREN = 0b0110,     // Write Enable Latch
        constexpr uint8_t OPCODE_WRDI = 0b0100,     // Reset Write Enable Latch
        constexpr uint8_t OPCODE_RDSR = 0b0101,     // Read Status Register
        constexpr uint8_t OPCODE_WRSR = 0b0001,     // Write Status Register
        constexpr uint8_t OPCODE_READ = 0b0011,     // Read Memory
        constexpr uint8_t OPCODE_WRITE = 0b0010,    // Write Memory
        constexpr uint8_t OPCODE_RDID = 0b10011111  // Read Device ID
        constexpr uint8_t OPCODE_SLEEP = 0b10111001 // Enter sleep mode
        constexpr uint8_t OPCODE_SNR = 0b11000011   // Read S/N

        void checkDeviceID() {
            bool validDeviceID = true;
            uint8_t expectedDeviceID = {0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0xC2, 0x24, 0x00} // Cypress FM25V10

            beginTransactionWithAssertingCS();
            _spi->transfer(OPCODE_RDID);
            for (uint8_t i = 0; i < 9; i++) {
                if (_spi->transfer(0xFF) != expectedDeviceID[i]) {
                    validDeviceID = false;
                }
            }
            endTransactionWithDeassertingCS();

            return validDeviceID;
        }

        void beginTransactionWithAssertingCS() {
            _spi->beginTransaction(*_spiSetting);
            digitalWrite(FramChipSelectPin, LOW);
        }

        void endTransactionWithDeassertingCS() {
            digitalWrite(FramChipSelectPin, HIGH);
            _spi->endTransaction();
        }
};
