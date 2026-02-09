#include <util/delay.h>
#include "tm1637.h"

void tm1637_init(TM1637 *tm) {
    io_out(&tm->dio);
    io_high(&tm->dio);

    io_out(&tm->clk);
    io_high(&tm->clk);
}

TM1637 tm1637_new(IO *clk, IO *dio) {
    TM1637 r = {
        .clk = *clk,
        .dio = *dio
    };

    tm1637_init(&r);

    return r;
}

void tm_delay_us(uint8_t us) {
    while (us--) {
        _delay_us(1);
    }
}

void dio_low(TM1637 *tm) {
    io_out(&tm->dio);
    io_low(&tm->dio);
}

void dio_high(TM1637 *tm) {
    io_out(&tm->dio);
    io_high(&tm->dio);
}

void dio_input_pullup(TM1637 *tm) {
    io_in(&tm->dio);
    io_high(&tm->dio);
}

void clk_low(TM1637 *tm) {
    io_out(&tm->clk);
    io_low(&tm->clk);
}

void clk_high(TM1637 *tm) {
    io_out(&tm->clk);
    io_high(&tm->clk);
}

void tm_start(TM1637 *tm) {
    clk_high(tm);
    dio_high(tm);
    tm_delay_us(20);

    dio_low(tm);
    tm_delay_us(20);

    clk_low(tm);
    tm_delay_us(20);
}

void tm_stop(TM1637 *tm) {
    clk_low(tm);
    tm_delay_us(20);

    dio_low(tm);
    tm_delay_us(20);

    clk_high(tm);
    tm_delay_us(20);

    dio_high(tm);
    tm_delay_us(20);
}

void tm_write_byte(TM1637 *tm, uint8_t b) {
    for (uint8_t i = 0; i < 8; i++) {
        clk_low(tm);
        tm_delay_us(20);

        if (b & 0x01) {
            dio_high(tm);
        } else {
            dio_low(tm);
        }

        tm_delay_us(20);
        clk_high(tm);
        tm_delay_us(20);

        b >>= 1;
    }

    /* ACK cycle */
    clk_low(tm);
    tm_delay_us(20);

    dio_input_pullup(tm);
    tm_delay_us(20);

    clk_high(tm);
    tm_delay_us(20);

    uint8_t ack = (io_read_bit(&tm->dio) == 0) ? 1 : 0;

    clk_low(tm);
    tm_delay_us(20);

    dio_high(tm);
    tm_delay_us(20);

}

uint8_t g_brightness = 7; /* 0..7 */

void tm_set_brightness(TM1637 *tm, uint8_t brightness, uint8_t display_on)
{
    g_brightness = brightness & 0x07;

    tm_start(tm);
    tm_write_byte(tm, (uint8_t)(0x80 | (display_on ? 0x08 : 0x00) | g_brightness));
    tm_stop(tm);
}

void tm_display_rawN(TM1637 *tm, uint8_t n, const uint8_t *segs)
{
    uint8_t i;

    /* data cmd: auto increment */
    tm_start(tm);
    tm_write_byte(tm, 0x40);
    tm_stop(tm);

    /* address cmd: start at 0 */
    tm_start(tm);
    tm_write_byte(tm, 0xC0);
    for (i = 0; i < n; i++) {
        tm_write_byte(tm, segs[i]);
    }
    tm_stop(tm);

    /* display control */
    tm_start(tm);
    tm_write_byte(tm, (uint8_t)(0x80 | 0x08 | (g_brightness & 0x07)));
    tm_stop(tm);
}

const uint8_t MAP6[6] = { 2, 1, 0, 5, 4, 3 };

void tm_display_raw6(TM1637 *tm, const uint8_t logical[6])
{
    uint8_t phys[6];
    uint8_t i;

    for (i = 0; i < 6; i++) {
        phys[i] = logical[MAP6[i]];
    }

    tm_display_rawN(tm, 6, phys);
}

void tm_clear6(TM1637 *tm)
{
    uint8_t segs[6] = {0,0,0,0,0,0};
    tm_display_raw6(tm, segs);
}
