#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

typedef struct {
    uint8_t current;
    uint8_t previous;

    uint8_t press;
    uint8_t release;

    uint8_t held;
    uint32_t time[8];
} keyboard_t;

keyboard_t keyboard_new();

void keyboard_process(keyboard_t *k, uint8_t value);

#endif
