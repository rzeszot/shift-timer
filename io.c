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

IO io_new_pb0() {
    return io_new(&DDRB, &PORTB, &PINB, PB0);
}

IO io_new_pb1() {
    return io_new(&DDRB, &PORTB, &PINB, PB1);
}

IO io_new_pb2() {
    return io_new(&DDRB, &PORTB, &PINB, PB2);
}

IO io_new_pb3() {
    return io_new(&DDRB, &PORTB, &PINB, PB3);
}

IO io_new_pb4() {
    return io_new(&DDRB, &PORTB, &PINB, PB4);
}

IO io_new_pb5() {
    return io_new(&DDRB, &PORTB, &PINB, PB5);
}

IO io_new_pb6() {
    return io_new(&DDRB, &PORTB, &PINB, PB6);
}

IO io_new_pb7() {
    return io_new(&DDRB, &PORTB, &PINB, PB7);
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

IO io_new_pc4() {
    return io_new(&DDRC, &PORTC, &PINC, PC4);
}

IO io_new_pc5() {
    return io_new(&DDRC, &PORTC, &PINC, PC5);
}

IO io_new_pc6() {
    return io_new(&DDRC, &PORTC, &PINC, PC6);
}


IO io_new_pd0() {
    return io_new(&DDRD, &PORTD, &PIND, PD0);
}

IO io_new_pd1() {
    return io_new(&DDRD, &PORTD, &PIND, PD1);
}

IO io_new_pd2() {
    return io_new(&DDRD, &PORTD, &PIND, PD2);
}

IO io_new_pd3() {
    return io_new(&DDRD, &PORTD, &PIND, PD3);
}

IO io_new_pd4() {
    return io_new(&DDRD, &PORTD, &PIND, PD4);
}

IO io_new_pd5() {
    return io_new(&DDRD, &PORTD, &PIND, PD5);
}

IO io_new_pd6() {
    return io_new(&DDRD, &PORTD, &PIND, PD6);
}

IO io_new_pd7() {
    return io_new(&DDRD, &PORTD, &PIND, PD7);
}
