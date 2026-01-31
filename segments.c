#include <stdint.h>
#include "segments.h"

const uint8_t segments_digit[10] = {
    0x3f, // 0
    0x06, // 1
    0x5b, // 2
    0x4f, // 3
    0x66, // 4
    0x6d, // 5
    0x7d, // 6
    0x07, // 7
    0x7f, // 8
    0x6f  // 9
};

const uint8_t segment_dot = 0x80;

uint8_t segment_for_character(char c) {
    uint8_t seg = 0x00;

    if (c >= '0' && c <= '9') {
        seg = segments_digit[c - '0'];
    } else if (c == ' ') {
        seg = 0x00;
    } else {
        seg = 0x00;
    }

    return seg;
}
