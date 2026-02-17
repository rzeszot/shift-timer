#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <avr/io.h>

#define KEY_ESCAPE  (1 << PB0)
#define KEY_BACK    (1 << PB1)
#define KEY_DOWN    (1 << PB2)
#define KEY_UP      (1 << PB3)
#define KEY_ENTER   (1 << PB4)

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
