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
    uint8_t flags;
    uint8_t _reserved_0;
    uint16_t _reserved_1;
    uint32_t _reserved_2;
    uint8_t title_length;
    char title[257];
};

struct __attribute__((packed)) chip_header {
    char magic_number[4];
    uint32_t packet_length;
    uint16_t chip_type;
    uint16_t bank_number;
    uint16_t load_address;
    uint16_t size;
};

/*!
 * \brief Load header information from a .chf file
 * 
 * \param file The file to load the header from
 * \param header The header struct to load the data into
 */
void get_chf_header(File& file, chf_header& header) {
    file.read((uint8_t*) &header, sizeof(chf_header) - sizeof(((chf_header){0}).title));
    file.read((uint8_t*) &header.title, header.title_length + 1);
    file.seek(header.header_length, SeekSet); // Skip padding
}

/*! \brief Load a program from the CHF File into program_rom
 * 
 * \param file the CHF file to load
 */
void read_chf_file(File &file) {
    // It's guaranteed the first 16 bytes are valid & the file size is >= 64 (file_header[48] + chip_header[16])

    // Read header
    chf_header header;
    get_chf_header(file, header);

    // Read chip packets
    chip_header ch;
    size_t header_start = file.position();
    file.read((uint8_t*) &ch, sizeof(ch));
    while (strncmp(ch.magic_number, "CHIP", 4) == 0) {
        // Set attribute and pull data
        memset(program_attribute + ch.load_address, ch.chip_type, ch.size);
        size_t chip_types_length = sizeof(ChipTypes) / sizeof(ChipTypes[0]);
        if (ch.chip_type < chip_types_length && ChipTypes[ch.chip_type]->has_data()) {
            file.read((uint8_t*) (program_rom + ch.load_address), ch.size);
            file.seek(header_start + ch.packet_length, SeekSet); // Skip padding
        }
        // Next packet
        if ((file.size() - file.position()) >= 16) {
            header_start = file.position();
            file.read((uint8_t*) &ch, sizeof(ch));
        } else {
            break;
        }
    }
}

/*!
 * \brief Get the filetype of a file object
 * 
 * \param file The file to check
 * \return The file type as a 32-bit constant
 */
uint32_t get_filetype(File& file) {
    if (file.isDirectory()) {
        return 'dir';
    } else {
        uint8_t magic_buffer[17] = {0};
        file.read((uint8_t*) magic_buffer, 1); // Read up to 1 byte into magic_buffer
        file.seek(0, SeekSet);

        switch (magic_buffer[0]) {
            case 'C':
                file.read((uint8_t*) magic_buffer, 16);                   // Read 16 bytes into magic_buffer
                file.seek(0, SeekSet);
                if (file.size() >= 64 && !strcmp((char*) magic_buffer, "CHANNEL F       ")) {
                    return 'chf';
                }
                break;
            case 0x55:
                return 'bin';
        }
        return '???';
    }
}

/*!
 * \brief Load a bin/chf file into memory
 * 
 * \param file The file to load
 */
void __not_in_flash_func(load_game)(File &file) {   // FIXME: Refactor to use get_filetype
    for (uint16_t i = 0; i <= 0xFF; i++) { // Unload IOPorts
       delete IOPorts[i];
       IOPorts[i] = nullptr;
    }

    // Clear program memory/attribute
    memset(program_attribute, RESERVED_CT::id, 0x10000);
    memset(program_rom, 0xFF, 0x10000);
    

    if (file) {
        uint8_t magic_buffer[17] = {0};
        file.read((uint8_t*) magic_buffer, 1); // Read up to 1 byte into magic_buffer
        if (magic_buffer[0] == 0x55) {         // .bin file

            // Assume hardware type 2 (ROM+RAM) with 2K of RAM at 0x2800
            if (file.size() > 0) {
                memset(program_attribute + 0x801, ROM_CT::id, min(file.size(), 0xF7FF)-1);
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
            file.seek(0, SeekSet);
            file.read((uint8_t*) (program_rom + 0x800), min(file.size(), 0xF7FF)); // Read up to 62K into program_rom

            // sleep_ms(100);
        } else if (magic_buffer[0] == 'C' && file.size() >= 64) {        // Possible .chf file
            file.seek(0, SeekSet);
            file.read((uint8_t*) magic_buffer, 16);                      // Read 16 bytes into magic_buffer
            if (strcmp((char*) magic_buffer, "CHANNEL F       ") == 0) { // .chf file
                file.seek(0, SeekSet);
                read_chf_file(file);
            }
        }
        file.close();
    } else {
        blink_code(BLINK::NO_VALID_FILES);
    }
}

/*! 
 * \brief Fetch the directory/program title
 * 
 * \param file The file to check
 * \param dest_title A char buffer (at least FILENAME_LIMIT in size) for the resulting title
 */
void get_program_title(File& file, char* dest_title) {
    char* filename = (char*)file.name();
    char* last_dot;

    switch (get_filetype(file)) {
        case 'dir':
            dest_title[0] = '/';
            translate_utf8_to_cp437(filename, dest_title + 1, FILENAME_LIMIT - 1);
            break;
        case 'chf':
            chf_header header;
            get_chf_header(file, header);
            file.seek(0, SeekSet);   // FIXME: remove
            translate_utf8_to_cp437(header.title, dest_title, FILENAME_LIMIT);
            break;
        case 'bin':
            last_dot = strrchr(filename, '.');   // Strip file extension
            translate_utf8_to_cp437(filename, dest_title, last_dot ? min(last_dot - filename, FILENAME_LIMIT) : FILENAME_LIMIT);
            break;
        default:
            translate_utf8_to_cp437(filename, dest_title, FILENAME_LIMIT);
    }
}