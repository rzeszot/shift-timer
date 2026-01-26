#include "io.h"

void io_high(IO *io) {
    *(io->port) |= io->mask;
}

void io_low(IO *io) {
    *(io->port) &= ~io->mask;
}

void io_out(IO *io) {
    *(io->ddr) |= io->mask;
}

void io_in(IO *io) {
    *(io->ddr) &= ~io->mask;
}

uint8_t io_read_bit(IO *io) {
    uint8_t value = *(io->pin) & io->mask;
    return value ? 1 : 0;
}

void io_init(IO *io) {
    *(io->ddr) |= io->mask;
}

IO io_new(volatile uint8_t *ddr, volatile uint8_t *port, volatile uint8_t *pin, uint8_t mask) {
    IO io = {
        .ddr = ddr,
        .port = port,
        .pin = pin,
        .mask = _BV(mask)
    };

    io_init(&io);

    return io;
}

IO io_new_pc0() {
    return io_new(&DDRC, &PORTC, &PINC, PC0);
}

IO io_new_pc1() {
    return io_new(&DDRC, &PORTC, &PINC, PC1);
}

IO io_new_pc2() {
    return io_new(&DDRC, &PORTC, &PINC, PC2);
}

IO io_new_pc3() {
    return io_new(&DDRC, &PORTC, &PINC, PC3);
}
