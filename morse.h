/* 
  morse.h - A library for flashing Morse code through the built in LED. Useful for debugging with more helpful error codes.
  Created April 20, 2022 by Jefferson A.
  Released into the public domain.

  Handy international Morse code table
         A .-             U ..-
         B -...           V ...-
         C -.-.           W .--
         D -..            X -..-
         E .              Y -.--
         F ..-.           Z --..
         G --.
         H ....
         I ..
         J .---
         K -.-            1 .----
         L .-..           2 ..---
         M --             3 ...--
         N -.             4 ....-
         O ---            5 .....
         P .--.           6 -....
         Q --.-           7 --...
         R .-.            8 ---..
         S ...            9 ----.
         T -              0 -----
 */

#ifndef Morse_H
#define Morse_H

#include "Arduino.h"

class Morse {
    public:
        Morse() {};
        void blink(uint8_t count) { // expects a value between 1 & 7
            this->parseMorse(String('9' + min(count + 1, 8)));
        }
        void parseMorse(String message) {
            for(char letter : message) {
                letter = toupper(letter);
                if ('0' <= letter && letter <= 'Z') {
                    uint8_t beep_code = internationalMorseCodeMap[letter - '0'];
                    do {
                        digitalWrite(LED_BUILTIN, HIGH);
                        delay((beep_code & 1) ? DASH : DOT);
                        digitalWrite(LED_BUILTIN, LOW);
                        delay(SYMBOL_SPACE);
                    } while ((beep_code >>= 1) != 1);    
                    delay(LETTER_SPACE);
                } else if (letter == ' ') {
                    delay(WORD_SPACE);     
                }     
            }
        }
    private:
        static constexpr uint16_t TIME_UNIT = 500;
        static constexpr uint16_t DOT = TIME_UNIT;                                // 1
        static constexpr uint16_t DASH = 3 * TIME_UNIT;                           // 111
        static constexpr uint16_t SYMBOL_SPACE = TIME_UNIT;                       // 0
        static constexpr uint16_t LETTER_SPACE = 3 * TIME_UNIT - SYMBOL_SPACE;    // 000
        static constexpr uint16_t WORD_SPACE = 7 * TIME_UNIT - LETTER_SPACE;      // 0000000
        
        //1 = dash symbol, 0 = dot symbol, MSB = end symbol. Read right to left
        static constexpr uint8_t internationalMorseCodeMap[] = {
            0x3f, 0x3e, 0x3c, 0x38, 0x30, 0x20, 0x21, 0x23, 0x27, 0x2f, 0x02, 0x04, 0x08,
            0x10, 0x20, 0x40, 0x80, 0x06, 0x11, 0x15, 0x09, 0x02, 0x14, 0x0b, 0x10, 0x04,
            0x1e, 0x0d, 0x12, 0x07, 0x05, 0x0f, 0x16, 0x1b, 0x0a, 0x08, 0x03, 0x0c, 0x18,
            0x0e, 0x19, 0x1d, 0x13
        };
};

#endif
