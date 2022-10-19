// 00xx | 01xx Table
inline constexpr uint8_t TABLE_00_01_MIN = 0x92;
inline constexpr uint8_t TABLE_00_01_MAX = 0xff;
uint8_t table_00_01[110] = {
    0x9f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xff, 0xad,
    0x9b, 0x9c, 0x3f, 0x9d, 0x3f, 0x3f, 0x3f, 0x3f,
    0xa6, 0xae, 0xaa, 0x3f, 0x3f, 0x3f, 0xf8, 0xf1,
    0xfd, 0x3f, 0x3f, 0xe6, 0x3f, 0xfa, 0x3f, 0x3f,
    0xa7, 0xaf, 0xac, 0xab, 0x3f, 0xa8, 0x3f, 0x3f,
    0x3f, 0x3f, 0x8e, 0x8f, 0x92, 0x80, 0x3f, 0x90,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xa5,
    0x3f, 0x3f, 0x3f, 0x3f, 0x99, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x9a, 0x3f, 0x3f, 0xe1, 0x85, 0xa0,
    0x83, 0x3f, 0x84, 0x86, 0x91, 0x87, 0x8a, 0x82,
    0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b, 0x3f, 0xa4,
    0x95, 0xa2, 0x93, 0x3f, 0x94, 0xf6, 0x3f, 0x97,
    0xa3, 0x96, 0x81, 0x3f, 0x3f, 0x98
};
// 03xx Table
inline constexpr uint8_t TABLE_03_MIN = 0x93;
inline constexpr uint8_t TABLE_03_MAX = 0xc6;
uint8_t table_03[52] = {
    0xe2, 0x3f, 0x3f, 0x3f, 0x3f, 0xe9, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0xe4, 0x3f, 0x3f, 0xe8, 0x3f, 0x3f, 0xea, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xe0, 0x3f,
    0x3f, 0xeb, 0xee, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xe3, 0x3f, 0x3f,
    0xe5, 0xe7, 0x3f, 0xed
};
// 20xx | 25xx Table
inline constexpr uint8_t TABLE_20_25_MIN = 0x0;
inline constexpr uint8_t TABLE_20_25_MAX = 0xa7;
uint8_t table_20_25[168] = {
    0xc4, 0x3f, 0xb3, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0xda, 0x3f, 0x3f, 0x3f,
    0xbf, 0x3f, 0x3f, 0x3f, 0xc0, 0x3f, 0x3f, 0x3f,
    0xd9, 0x3f, 0x3f, 0x3f, 0xc3, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0xb4, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0xc2, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0xc1, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0xc5, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0xcd, 0xba, 0xd5, 0xd6, 0xc9, 0xb8, 0xb7, 0xbb,
    0xd4, 0xd3, 0xc8, 0xbe, 0xbd, 0xbc, 0xc6, 0xc7,
    0xcc, 0xb5, 0xb6, 0xb9, 0xd1, 0xd2, 0xcb, 0xcf,
    0xd0, 0xca, 0xd8, 0xd7, 0xce, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xfc,
    0xdf, 0x3f, 0x3f, 0x3f, 0xdc, 0x3f, 0x3f, 0x3f,
    0xdb, 0x3f, 0x3f, 0x3f, 0xdd, 0x3f, 0x3f, 0x3f,
    0xde, 0xb0, 0xb1, 0xb2, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0xfe, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x9e
};
// 22xx | 23xx Table
inline constexpr uint8_t TABLE_22_23_MIN = 0x10;
inline constexpr uint8_t TABLE_22_23_MAX = 0x65;
uint8_t table_22_23[86] = {
    0xa9, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0xf9, 0xfb, 0x3f, 0x3f, 0x3f, 0xec, 0x3f,
    0xf4, 0xf5, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0xef, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0xf7, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0xf0, 0x3f, 0x3f, 0xf3, 0xf2
};

uint8_t get_CP437(uint16_t unicode_char) {
    uint8_t unicode_char_l = unicode_char & 0xFF;
    switch (unicode_char >> 8) {
        case 0x00:
            if (unicode_char_l <= 0x7F) {
                return unicode_char_l;
            }
            [[fallthrough]];
        case 0x01:
            if (TABLE_00_01_MIN <= unicode_char_l && unicode_char_l <= TABLE_00_01_MAX) {
                return table_00_01[unicode_char_l - TABLE_00_01_MIN];
            }
            return '?';
        case 0x03:
            if (TABLE_03_MIN <= unicode_char_l && unicode_char_l <= TABLE_03_MAX) {
                return table_03[unicode_char_l - TABLE_03_MIN];
            }
            return '?';
        case 0x20:
        case 0x25:
            if (TABLE_20_25_MIN <= unicode_char_l && unicode_char_l <= TABLE_20_25_MAX) {
                return table_20_25[unicode_char_l - TABLE_20_25_MIN];
            }
            return '?';
        case 0x22:
        case 0x23:
            if (TABLE_22_23_MIN <= unicode_char_l && unicode_char_l <= TABLE_22_23_MAX) {
                return table_22_23[unicode_char_l - TABLE_22_23_MIN];
            }
            [[fallthrough]];
        default:
            return '?';
    }
}

uint16_t get_unicode(uint8_t* source, uint16_t& i) {
    uint8_t leader = source[i++];
    if (leader < 0b10000000) {        // 0xxxxxxx; One byte
        return leader;
    } else if (leader < 0b11000000) { // 10xxxxxx; Invalid leading byte
        return -1;
    } else if (leader < 0b11100000) { // 110xxxxx; Two bytes
        return ((leader & 0x1f) << 6) | (source[i++] & 0x3f);
    } else if (leader < 0b11110000) { // 1110xxxx; Three bytes
        return ((leader & 0xf) << 12) | ((source[i++] & 0x3f) << 6) | (source[i++] & 0x3f);
    } else if (leader < 0b11111000) { // 11110xxx; Four bytes
        return -1;
    } else {                          // Invalid leading byte
        return -1;
    }
}

void translate_utf8_to_cp437(char* source, char* destination, uint8_t destination_limit) {
    for (uint16_t src_i = 0, dest_i = 0; src_i < strlen(source) && dest_i < destination_limit;) {
        destination[dest_i++] = (char)get_CP437(get_unicode((uint8_t*)source, src_i));
    }
}