#include "buzz.h"
#include <avr/io.h>

void buzz_set(uint8_t enabled) {
    DDRC |= (1 << PC5);

    if (!enabled) {
        PORTC &= ~(1 << PC5);
    } else {
        PORTC |= (1 << PC5);
    }
}

