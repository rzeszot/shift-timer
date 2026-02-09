#include <avr/io.h>
#include "keyboard.h"

#define LONG_PRESS_MS 500

void keyboard_init(keyboard_t *k) {

}

keyboard_t keyboard_new() {
    keyboard_t k= {
        .current = 0,
        .previous = 0,
        .press = 0,
        .release = 0,
        .held = 0,
        .time = { 0 }
    };

    keyboard_init(&k);

    return k;
}

void keyboard_process(keyboard_t *r, uint8_t value, uint32_t delta_ms) {
    r->current = value;

    r->press   =  r->current & ~r->previous;
    r->release = ~r->current &  r->previous;

    for (uint8_t i = 0; i < 8; i++) {
        if (r->press & _BV(i)) {
            r->time[i] = 0;
            r->held &= ~_BV(i);
        }
    }

    for (uint8_t i = 0; i < 8; i++) {
        if (r->current & _BV(i)) {
            r->time[i] += delta_ms;

            if (r->time[i] >= LONG_PRESS_MS && !(r->held & _BV(i))) {
                r->held |= _BV(i);
            }
        }
    }

    for (uint8_t i = 0; i < 8; i++) {
        if (r->release & _BV(i)) {
            r->time[i] = 0;
            r->held &= ~_BV(i);
        }
    }

    r->previous = r->current;
}
