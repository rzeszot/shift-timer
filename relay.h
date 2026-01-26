#ifndef RELAY_H
#define RELAY_H

#include <avr/io.h>
#include <stdint.h>
#include "io.h"

typedef struct {
    IO io;
} Relay;

void relay_init(Relay *r);

void relay_on(Relay *r);
void relay_off(Relay *r);
void relay_set(Relay *r, uint8_t on);

Relay relay_new(IO *io);

#endif
