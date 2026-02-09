#include "button.h"

void button_init(Button *b) {
    io_in(&b->io);
    io_high(&b->io);
}

uint8_t button_read(Button *b) {
    return io_read_bit(&b->io);
}

Button button_new(IO *io) {
    Button r = {
        .io = *io
    };
    button_init(&r);
    return r;
}
