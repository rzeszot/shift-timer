#ifndef IO_H
#define IO_H

#include <avr/io.h>
#include <stdint.h>

typedef struct {
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    volatile uint8_t *pin;
    uint8_t mask;
} IO;

void io_high(IO *io);
void io_low(IO *io);

void io_out(IO *io);
void io_in(IO *io);

uint8_t io_read_bit(IO *io);

IO io_new(volatile uint8_t *ddr, volatile uint8_t *port, volatile uint8_t *pin, uint8_t mask);

IO io_new_pc0();
IO io_new_pc1();
IO io_new_pc2();
IO io_new_pc3();

#endif
