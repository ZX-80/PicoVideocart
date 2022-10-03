/** \file file_cache.hpp
 * 
 * \brief Handles most code related to loading ROMS
 */

#pragma once

inline constexpr uint16_t FOLDER_LIMIT = 100; // Max files displayed per folder
inline uint16_t DIR_LIMIT = 0;                // Max index into file_data

struct __attribute__((packed)) file_info {
    char title[32];
    bool isFile;
};

// Use go terms for slice size / used
inline file_info file_data[FOLDER_LIMIT] = {0};

void string_copy(char* destination, char* source, uint8_t size, bool write_null=false, char pad_char=' ');

__force_inline void string_copy(char* destination, char* source, uint8_t size, bool write_null, char pad_char) {
    size_t source_len = strlen(source);
    for (uint16_t i = 0; (i < size) && (i < source_len); i++) {
        destination[i] = source[i];
    }
    for (uint16_t i = source_len; i < size; i++) {
        destination[i] = pad_char;
    }
    if (write_null) {
        destination[size] = '\0';
    }
}