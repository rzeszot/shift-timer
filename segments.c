#include <stdint.h>
#include "segments.h"

const uint8_t segments_digit[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
const uint8_t segment_dot = 0x80;

uint8_t segment_for_int(uint8_t value) {
    if (value < 10) {
        return segments_digit[value];
    }
    return 0x49;
}

uint8_t segment_for_character(char c) {
    if (c >= '0' && c <= '9') {
        return segments_digit[(uint8_t)(c - '0')];
    } else if (c == ' ') {
        return 0x00;
    } else if (c == '-') {
        return 0x40;
    } else {
        return 0x49;
    }
}
