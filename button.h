#ifndef BUTTON_H
#define BUTTON_H

#include <avr/io.h>
#include <stdint.h>
#include "io.h"

typedef struct {
    IO io;
} Button;

uint8_t button_read(Button *b);

Button button_new(IO *io);

#endif
