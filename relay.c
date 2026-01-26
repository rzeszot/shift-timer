#include "relay.h"

void relay_on(Relay *r) {
    io_low(&r->io);
}

void relay_off(Relay *r) {
    io_high(&r->io);
}

void relay_set(Relay *r, uint8_t on) {
    if (on) {
        relay_on(r);
    } else {
        relay_off(r);
    }
}

void relay_init(Relay *r) {
    *(r->io.port) |= r->io.mask;
}

Relay relay_new(IO *io) {
    Relay r = {
        .io = *io
    };
    relay_init(&r);

    return r;
}
