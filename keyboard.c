#include <avr/io.h>
#include "keyboard.h"

#define LONG_PRESS_MS_STAGE1 200
#define LONG_PRESS_MS_STAGE2 500
#define LONG_PRESS_MS_STAGE3 1000
#define LONG_PRESS_MS_STAGE4 1500
#define HELD_EVENT_EVERY_STAGE1 20
#define HELD_EVENT_EVERY_STAGE2 5
#define HELD_EVENT_EVERY_STAGE3 2
#define HELD_EVENT_EVERY_STAGE4 1

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

void keyboard_process(keyboard_t *r, uint8_t value) {
    uint8_t delta_ms = 1;

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

            if (r->time[i] >= LONG_PRESS_MS_STAGE1) {
                uint16_t since_threshold = r->time[i] - LONG_PRESS_MS_STAGE1;
                uint16_t interval;

                if (r->time[i] >= LONG_PRESS_MS_STAGE4) {
                    interval = HELD_EVENT_EVERY_STAGE4;
                } else if (r->time[i] >= LONG_PRESS_MS_STAGE3) {
                    interval = HELD_EVENT_EVERY_STAGE3;
                } else if (r->time[i] >= LONG_PRESS_MS_STAGE2) {
                    interval = HELD_EVENT_EVERY_STAGE2;
                } else {
                    interval = HELD_EVENT_EVERY_STAGE1;
                }

                if ((since_threshold % interval) == 0) {
                    r->held |= _BV(i);
                } else {
                    r->held &= ~_BV(i);
                }
            } else {
                r->held &= ~_BV(i);
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

