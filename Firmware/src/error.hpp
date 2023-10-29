/** \file error.hpp
 *
 * \brief Blink code functionality (same concept as beep codes)
 */

#pragma once

namespace BLINK {
    inline constexpr uint8_t OVERCLOCK_FAILED = 3;
    inline constexpr uint8_t NO_VALID_FILES = 4;
}

/*! \brief Blink an error code to the LED. Useful for simple debugging
 *
 * \param code The error code
 * \param repeat the amount of times to output the code
 */
void blink_code(uint8_t code, uint8_t repeat=3) {
    digitalWriteFast(LED_BOARD, LOW);
    delay(1000);
    for (uint8_t j = 0; j < repeat; j++) {
        for (uint8_t i = 0; i < 2 * code; i++) {
            digitalToggleFast(LED_BOARD);
            delay(250);
        }
        delay(1000);
    }
}