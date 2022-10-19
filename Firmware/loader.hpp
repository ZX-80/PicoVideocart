/** \file loader.hpp
 * 
 * \brief Handles loading both .bin and .chf ROM files
 * 
 * \details BIN files are just raw chunks of ROM that can be loaded directly
 * into memory. CHF files are a special container specifically designed for
 * Channel F programs, providing all the necessary information to preserve
 * and load it.
 * 
 * Refer to the [CHF repository](https://github.com/ZX-80/Videocart-Image-Format)
 * for more information.
 */

#pragma once

#include "unicode.hpp"
#include "chips.hpp"
#include "error.hpp"
#include "ports.hpp"

#include <SPI.h>
#include <SD.h>

struct __attribute__((packed)) chf_header {
    char magic_number[16];
    uint32_t header_length;
    uint8_t minor_version;
    uint8_t major_version;
    uint16_t hardware_type;
    uint64_t reserved;
    uint8_t title_length;
};
struct __attribute__((packed)) chip_header {
    char magic_number[4];
    uint32_t packet_length;
    uint16_t chip_type;
    uint16_t bank_number;
    uint16_t load_address;
    uint16_t size;
};

/*! \brief Load a program from the CHF File into program_rom
 * 
 * \param romFile the CHF file to load
 */
void read_chf_file(File &romFile) {
    // It's guaranteed the first 16 bytes are valid & the file size is >= 64 (file_header[48] + chip_header[16])

    // Read header
    chf_header header;
    romFile.read((uint8_t*) &header, sizeof(chf_header));

    // Read title
    char title[257] = {0};
    romFile.read((uint8_t*) &title, header.title_length + 1);
    romFile.seek(header.header_length, SeekSet); // Skip padding

    // Read chip packets
    chip_header ch;
    size_t header_start = romFile.position();
    romFile.read((uint8_t*) &ch, sizeof(ch));
    while (strncmp(ch.magic_number, "CHIP", 4) == 0) {
        // Set attribute and pull data
        memset(program_attribute + ch.load_address, ch.chip_type, ch.size);
        size_t chip_types_length = sizeof(ChipTypes) / sizeof(ChipTypes[0]);
        if (ch.chip_type < chip_types_length && ChipTypes[ch.chip_type]->has_data()) {
            romFile.read((uint8_t*) (program_rom + ch.load_address), ch.size);
            romFile.seek(header_start + ch.packet_length, SeekSet); // Skip padding
        }
        
        // Next packet
        if ((romFile.size() - romFile.position()) >= 16) {
            header_start = romFile.position();
            romFile.read((uint8_t*) &ch, sizeof(ch));
        } else {
            break;
        }
    }
}

void __not_in_flash_func(load_game)(File &romFile) {
    for (uint16_t i = 0; i <= 0xFF; i++) { // Unload IOPorts
       delete IOPorts[i];
       IOPorts[i] = nullptr;
    }

    // TODO: should probably zero the following (although writing to undefined memory could simply be undefined)
    // program_attribute
    // program_rom
    memset(program_attribute, RESERVED_CT::id, 0x10000);
    memset(program_rom, 0xFF, 0x10000);
    

    if (romFile) {
        uint8_t magic_buffer[17] = {0};
        romFile.read((uint8_t*) magic_buffer, 1); // Read up to 1 byte into magic_buffer
        if (magic_buffer[0] == 0x55) { // .bin file

            // Assume hardware type 2 (ROM+RAM) with 2K of RAM at 0x2800
            if (romFile.size() > 0) {
                memset(program_attribute + 0x801, ROM_CT::id, min(romFile.size(), 0xF7FF)-1);
            }
            memset(program_rom + 0x2800, 0, 0x800); // Clear RAM
            memset(program_attribute + 0x2800, RAM_CT::id, 0x800);

            // Assume 2012 SRAM on ports $20/$21/$24/$25
            IOPorts[0x20] = new Sram2102(0);
            IOPorts[0x21] = new Sram2102(1);
            IOPorts[0x24] = new Sram2102(0);
            IOPorts[0x25] = new Sram2102(1);
            IOPorts[0xFF] = new Launcher(file_data);

            // Read up to 62K into program_rom
            romFile.seek(0, SeekSet);
            romFile.read((uint8_t*) (program_rom + 0x800), min(romFile.size(), 0xF7FF)); // Read up to 62K into program_rom

            // sleep_ms(100);
        } else if (magic_buffer[0] == 'C' && romFile.size() >= 64) {     // Possible .chf file
            romFile.seek(0, SeekSet);
            romFile.read((uint8_t*) magic_buffer, 16);                   // Read 16 bytes into magic_buffer
            if (strcmp((char*) magic_buffer, "CHANNEL F       ") == 0) { // .chf file
                romFile.seek(0, SeekSet);
                read_chf_file(romFile);
            }
        }
        romFile.close();
    } else {
        blink_code(BLINK::NO_VALID_FILES);
    }
}

// uint32_t get_filetype(File& file) {
//     if ... return 'bin';
//     else if ... return 'chf';
// }

// struct __attribute__((packed)) chf_header {
//     char magic_number[16];
//     uint32_t header_length;
//     uint8_t minor_version;
//     uint8_t major_version;
//     uint16_t hardware_type;
//     uint64_t reserved;
//     uint8_t title_length;
//     char title[257];
// };
// void get_chf_header(File& file, chf_header& header) {
//     romFile.read((uint8_t*) &header, sizeof(chf_header) - sizeof(((chf_header){0}).title));
//     romFile.read((uint8_t*) &header.title, header.title_length + 1);
//     romFile.seek(header.header_length, SeekSet); // Skip padding
// }

bool get_program_title(File& current_file, char* dest_title) {  // from filename or chf data (utf-8 formatted)
    if (current_file.isDirectory()) {
        translate_utf8_to_cp437((char*)current_file.name(), dest_title, FILENAME_LIMIT);
        return true;
    } else {
        uint8_t magic_buffer[17] = {0};
        current_file.read((uint8_t*) magic_buffer, 1); // Read up to 1 byte into magic_buffer
        current_file.seek(0, SeekSet);

        if (magic_buffer[0] == 'C' && current_file.size() >= 64) {     // Possible .chf file
            current_file.read((uint8_t*) magic_buffer, 16);                   // Read 16 bytes into magic_buffer
            current_file.seek(0, SeekSet);
            if (strcmp((char*) magic_buffer, "CHANNEL F       ") == 0) { // .chf file,  !strcmp vs strcmp==0
                
                // Read header
                chf_header header;
                current_file.read((uint8_t*) &header, sizeof(chf_header));

                // Read title
                char title[257] = {0};
                current_file.read((uint8_t*) &title, header.title_length + 1);
                current_file.seek(0, SeekSet);
                translate_utf8_to_cp437(title, dest_title, FILENAME_LIMIT);
                return true;
            }
        } else {
            uint16_t limit = FILENAME_LIMIT;
            const char* filename = current_file.name();   // Strip file extension
            if (magic_buffer[0] == 0x55) {                     // .bin file
                const char* last_dot = strrchr(filename, '.');   
                // uint16_t limit = FILENAME_LIMIT;
                // if (last_dot != NULL) {
                    // limit = min(last_dot - filename, FILENAME_LIMIT);
                    // filename[last_dot] = NULL;
                // }

                // limit = last_dot ? FILENAME_LIMIT : min(last_dot - filename, FILENAME_LIMIT)
                // translate_utf8_to_cp437(filename, dest_title, FILENAME_LIMIT);
                uint16_t limit = last_dot ? min(last_dot - filename, FILENAME_LIMIT) : FILENAME_LIMIT;
            }
            translate_utf8_to_cp437((char*)filename, dest_title, limit);
            return true;
        }
    }
    return false;
}