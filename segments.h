#ifndef SEGMENTS_H
#define SEGMENTS_H

#include <stdint.h>

uint8_t segment_for_character(char c);
uint8_t segment_for_int(uint8_t value);

extern const uint8_t segment_dot;

#endif
