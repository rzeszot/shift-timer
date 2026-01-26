#include <util/delay.h>
#include "tm.h"

void tm_init(TM *tm) {
    io_out(&tm->stb);
    io_out(&tm->clk);
    io_in(&tm->dio);

    io_high(&tm->stb);
    io_high(&tm->clk);
    io_high(&tm->dio);
}

TM tm_new(IO *stb, IO *clk, IO *dio) {
    TM r = {
        .stb = *stb,
        .clk = *clk,
        .dio = *dio
    };

    tm_init(&r);

    return r;
}

void tm_start(TM *tm) {
    io_out(&tm->dio);
    io_low(&tm->stb);

    tm_delay();
}

void tm_stop(TM *tm) {
    io_high(&tm->stb);

    tm_delay();

    io_in(&tm->dio);
    io_high(&tm->dio);
}

void tm_write_byte(TM *tm, uint8_t b) {
    io_out(&tm->dio);

    for (uint8_t i = 0; i < 8; i++) {
        io_low(&tm->clk);
        tm_delay();

        io_out(&tm->dio);

        if (b & 0x01) {
            io_high(&tm->dio);
        } else {
            io_low(&tm->dio);
        }

        tm_delay();
        io_high(&tm->clk);
        tm_delay();

        b >>= 1;
    }
}

void tm_write_data(TM *tm, uint8_t addr, const uint8_t *data, uint8_t len) {
    tm_cmd(tm, 0x40);
    tm_start(tm);
    tm_write_byte(tm, 0xC0 | (addr & 0x0F));

    for (uint8_t i = 0; i < len; i++) {
        tm_write_byte(tm, data[i]);
    }

    tm_stop(tm);
}

uint8_t tm_read_byte(TM *tm) {
    uint8_t b = 0;

    io_in(&tm->dio);
    io_high(&tm->dio);

    for (uint8_t i = 0; i < 8; i++) {
        io_low(&tm->clk);

        tm_delay();
        io_high(&tm->clk);
        tm_delay();
        if (io_read_bit(&tm->dio)) {
            b |= (1 << i);
        }
    }

    return b;
}

uint8_t tm_read_keys(TM *tm) {
    uint8_t keys = 0;
    tm_start(tm);
    tm_write_byte(tm, 0x42);

    for (uint8_t i = 0; i < 4; i++) {
        uint8_t v = tm_read_byte(tm);
        keys |= (v << i);
    }

    tm_stop(tm);
    return keys;
}

void tm_cmd(TM *tm, uint8_t c) {
    tm_start(tm);
    tm_write_byte(tm, c);
    tm_stop(tm);
}

void tm_set_brightness(TM *tm, uint8_t b) {
    if (b > 7) {
        b = 7;
    }

    tm_cmd(tm, 0x88 | b);
}

void tm_clear(TM *tm) {
    uint8_t zeros[16] = {0};
    tm_write_data(tm, 0, zeros, 16);
}

void tm_delay() {
    _delay_us(2);
}
