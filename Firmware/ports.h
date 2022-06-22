/** \file ports.h
 *  \brief Channel F I/O Ports
 *
 *  \details The Channel F has 256 addressable I/O ports that it communicates with via the
 *  OUT(S) and IN(S) instructions.
 *
 *  - Four of these ports are assigned to the CPU/PSU
 *    - https://channelf.se/veswiki/index.php?title=Port
 *  - Four can be found on the 3853 SMI IC
 *  - Two are used to connect a 2102 SRAM IC
 *  - The remaining addresses were never used by any official Channel F products
 *
 *  ### Default Port Assignments
 *
 *   Port Address   | Device           | Description
 *   ---------------|------------------|-------------
 *   0              | CPU              | buttons and Video RAM
 *   1              | CPU              | right controller and pixel palette
 *   4              | PSU              | left controller and horizontal video position
 *   5              | PSU              | sound and vertical video position
 *   6              | MK 3870/3871     | interrupt control port (differs from the 3853 ICP)
 *   7              | MK 3870/3871     | binary timer
 *   12             | 3853 SMI         | programmable interrupt vector (upper byte)
 *   13             | 3853 SMI         | programmable interrupt vector (lower byte)
 *   14             | 3853 SMI         | interrupt control port
 *   15             | 3853 SMI         | programmable timer
 *   36             | Videocart 10/18  | 2102 SRAM
 *   37             | Videocart 10/18  | 2102 SRAM
 *
 *  ### Experimental Port Devices
 * 
 *   Port Address   | Device           | Description
 *   ---------------|------------------|-------------
 *   8              | Flashcart        | Random number generator
 *   9              | Flashcart        | Data stack
 *   10             | Flashcart        | Return stack
 *
 */

#pragma once

#include "hardware/structs/rosc.h"

/** \brief Abstract base class for ports */
class IOPort {
    public:
        virtual uint8_t read() = 0;
        virtual void write(uint8_t) = 0;
};

/** \brief A mapping from addresses to I/O ports */
extern IOPort* IOPorts[256];

// TODO: 3853 SMI, MK 3870

/**
 * \brief Implementation of a 2102 SRAM IC
 *
 * \details The 2102 is an asynchronous 1024 x 1-bit static random access
 * read/write memory. It's only used in Videocart 10 (Maze) and 18 (Hangman).
 * Data is normally written when the `read/WRITE` pin is low, but because the
 * ports invert the data, we write when it's high.
 *
 * More info found at http://seanriddle.com/mazepat.asm or any 2102 SRAM
 * datasheet.
 *
 * ### Port Details
 *
 *  Bit | Port A | Port B
 *  ----|--------|--------
 *  7   | OUT    | A9
 *  6   | -      | A8
 *  5   | -      | A7
 *  4   | -      | A1
 *  3   | IN     | A0
 *  2   | A2     | A5
 *  1   | A3     | A4
 *  0   | RW     | A0
 *
 */
class Sram2102 : public IOPort {
    private:
        uint8_t portIndex;
        inline static bool sramData[1024];
        inline static uint8_t portA;
        inline static uint8_t portB;
        inline static uint16_t address;
        static constexpr uint8_t OUT_FLAG = 0x80;
        static constexpr uint8_t IN_FLAG = 0x8;
        static constexpr uint8_t ADDR_MASK = 0x6;
        static constexpr uint8_t WRITE_FLAG = 0x1;

    public:
        Sram2102(uint8_t portIndex): portIndex(portIndex) {}

        uint8_t read() {
            return portIndex ? portB : portA;
        }

        void write(uint8_t data) {
            if (portIndex) {
                portB = data;
            } else {
                portA = data & 0xF;
            }

            // Update DATA OUT
            address = (portA & ADDR_MASK) << 7 | portB;
            if (portA & WRITE_FLAG) {
                sramData[address] = portA & IN_FLAG;
            }
            portA = sramData[address] << 7 | portA & ~OUT_FLAG;
        }
};

/**
 * \brief An IO port pseudo random number generator
 *
 * \details This IO port provides decent random numbers using a fast
 * multiply-with-carry algorithm. The seed for this algorithm comes from
 * either the programmer, or the Pico's Ring Oscillator.
 *
 * Praise RNJesus
 *
 */
class Random : public IOPort {
    private:
        // Any non-zero 32-bit values
        uint32_t z = 362436069;
        uint32_t w = 521288629;

        // Must be two distinct 16-bit constants for which both k*2^16-1 and k*2^15-1 are prime
        static constexpr uint16_t Z_CONST = 30135;
        static constexpr uint16_t W_CONST = 18513;

        /**
         * \brief Generate a random number using multiply-with-carry (MWC)
         *
         * \return a random number
         *
         * \details This specific MWC algorithm was choosen for its simplicity,
         * high speed, long period (>2^60), and decent randomness properties.
         * It was designed by George Marsaglia and is detailed here:
         * https://www.math.uni-bielefeld.de/~sillke/ALGORITHMS/random/marsaglia-c
         *
         */
        uint32_t rand32Mwc() {
            z = Z_CONST * (z & 0xFFFF) + (z >> 16);
            w = W_CONST * (w & 0xFFFF) + (w >> 16);
            return (z << 16) + (w & 0xFFFF);
        }

        /**
         * \brief Generate a random number using the Pico's ring oscillator
         * (ROSC)
         *
         * \return a random number
         *
         * \details It's important to note that the ring oscillator is a
         * pretty poor source of random numbers on its own.
         * > It's not well characterised, somewhat biased, output is somewhat
         * > periodic when sampled rapidly, etc.
         * source: https://github.com/raspberrypi/pico-sdk/issues/569
         *
         */
        uint32_t rand32Rosc() {
            uint32_t weaklyRandomNum = 0;
            for (uint8_t i = 0; i < 32; i++) {
                weaklyRandomNum = (weaklyRandomNum << 1) | rosc_hw->randombit;
            }
            return weaklyRandomNum;
        }

        /**
         * \brief Generate a random number using the Pico's ring oscillator
         * (ROSC) and a randomness extractor
         *
         * \return a random number
         *
         * \details This version attempts to improve the ring oscillator's
         * randomness properties using a randomness extractor function. In
         * particular, it uses a 32-bit FNV-1a hash on 64-bits supplied by
         * the ring oscillator.
         *
         * More information on the FNV hash function can be found at:
         *   - https://datatracker.ietf.org/doc/html/draft-eastlake-fnv-17.html
         *   - http://www.isthe.com/chongo/tech/comp/fnv/index.html
         *
         */
        uint32_t rand32RoscExtractor() {
            uint8_t weaklyRandomByte = 0;
            uint32_t betterRandomNum = 0x811c9dc5; // FNV offset basis
            for (uint8_t i = 0; i < 8; i++) {
                for (uint8_t j = 0; j < 8; j++) {
                    weaklyRandomByte = (weaklyRandomByte << 1) | rosc_hw->randombit;
                }
                betterRandomNum ^= weaklyRandomByte;
                betterRandomNum *= 0x1000193; // FNV prime
            }
            return betterRandomNum;
        }

    public:
        Random(): z(rand32RoscExtractor()), w(rand32RoscExtractor()) {}

        /**
         * \brief Generate a pseudo random number
         *
         * \return A random byte
         */
        uint8_t read() {
            uint32_t rand32 = rand32Mwc();
            return (rand32 >> 8) ^ rand32; // XOR-fold 32-bits into 8-bits
        }

        /**
         * \brief Shift a byte into the 64-bit seed
         *
         * \param seed The byte to shift in
         *
         * \details The 64-bit seed value cannot be zero. Thus shifting in 8 zeros causes it to be re-seeded using the
         * Pico's ring oscillator. This can be used to switch between deterministic and non-deterministic RNG.
         *
         */
        void write(uint8_t seed) {
            // Shift in the byte
            z = (z << 8) | (w >> 24);
            w = (w << 8) | seed;

            if (w == 0) {                  // Re-seed
                w = rand32RoscExtractor(); // TODO: Is this too slow?
            }
        }
};

/**
 * \brief An IO port full descending hardware stack
 *
 * \details 
 * Stack pointer:   [Full stack], Empty stack
 * Stack direction: [Descending], Ascending
 *
 */
class HardwareStack : public IOPort {
    private:
        static constexpr uint16_t STACK_SIZE = 2048; // Must be a power of 2
        uint8_t stackData[STACK_SIZE];
        uint16_t stackPointer = STACK_SIZE - 1; // TODO: should the stack pointer be R/W-able on a 2nd port?

    public:
        HardwareStack() = default;

        uint8_t read() {
            return stackData[(stackPointer++) & (STACK_SIZE - 1)];
        }

        void write(uint8_t data) {
            stackData[(--stackPointer) & (STACK_SIZE - 1)] = data;
        }
};
