/** \file morse.h
 *  \brief A library for flashing Morse code through the built in LED.
 *  \author Jefferson A.
 *  \date Created April 20, 2022
 *  \copyright Released into the public domain.
 *  \details
 *  Handy international Morse code table
 *       A .-             U ..-
 *       B -...           V ...-
 *       C -.-.           W .--
 *       D -..            X -..-
 *       E .              Y -.--
 *       F ..-.           Z --..
 *       G --.
 *       H ....
 *       I ..
 *       J .---
 *       K -.-            1 .----
 *       L .-..           2 ..---
 *       M --             3 ...--
 *       N -.             4 ....-
 *       O ---            5 .....
 *       P .--.           6 -....
 *       Q --.-           7 --...
 *       R .-.            8 ---..
 *       S ...            9 ----.
 *       T -              0 -----
 */

#pragma once

#include "Arduino.h"

class Morse {
    public:
        Morse() {};
  
        /**
         * \brief Output a message via Morse code and the built-in LED
         *
         * \param message The string to output
         *
         * \details The Morse code for a letter is obtained using INTERNATIONAL_MORSE_CODE_MAP. The format is  
         * 1 = dash symbol, 0 = dot symbol, MSB = end symbol, and it's read right to left.
         * For example: 1011 is END, dot, dash, dash, which when read right to left is "--." or "G"
         * The array has 43 elements; A translation (ascii to Morse code) ranging from ascii 0x30 (0) to 0x5A (Z)
         */
        static void print(String message) {
            for(char character : message) {
                if (isdigit(character) || isalpha(character)) {
                    uint8_t beepCode = INTERNATIONAL_MORSE_CODE_MAP[toupper(character) - '0'];
                    do {
                        digitalWrite(LED_BUILTIN, HIGH);
                        delay((beepCode & 1) ? DASH : DOT);
                        digitalWrite(LED_BUILTIN, LOW);
                        delay(SYMBOL_SPACE);
                    } while ((beepCode >>= 1) != 1);    
                    delay(LETTER_SPACE - SYMBOL_SPACE);
                } else if (character == ' ') {
                    delay(WORD_SPACE - LETTER_SPACE);     
                }     
            }
        }
    private:
        static constexpr uint16_t TIME_UNIT = 500;                 // Timings: 1 = On, 0 = Off
        static constexpr uint16_t DOT = TIME_UNIT;                 // 1
        static constexpr uint16_t DASH = 3 * TIME_UNIT;            // 111
        static constexpr uint16_t SYMBOL_SPACE = TIME_UNIT;        // 0
        static constexpr uint16_t LETTER_SPACE = 3 * TIME_UNIT;    // 000
        static constexpr uint16_t WORD_SPACE = 7 * TIME_UNIT;      // 0000000
        static constexpr uint8_t INTERNATIONAL_MORSE_CODE_MAP[] = {
            0x3f, 0x3e, 0x3c, 0x38, 0x30, 0x20, 0x21, 0x23, 0x27, 0x2f, 0x02, 0x02, 0x02,
            0x02, 0x02, 0x02, 0x02, 0x06, 0x11, 0x15, 0x09, 0x02, 0x14, 0x0b, 0x10, 0x04,
            0x1e, 0x0d, 0x12, 0x07, 0x05, 0x0f, 0x16, 0x1b, 0x0a, 0x08, 0x03, 0x0c, 0x18,
            0x0e, 0x19, 0x1d, 0x13
        };
};
