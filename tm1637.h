#ifndef TM1637_H
#define TM1637_H

#include "io.h"

typedef struct {
    IO clk;
    IO dio;
} TM1637;

TM1637 tm1637_new(IO *clk, IO *dio);

void tm_delay_us(uint8_t us);

void tm_start(TM1637 *tm);
void tm_stop(TM1637 *tm);

void tm_write_byte(TM1637 *tm, uint8_t b);


void tm_set_brightness(TM1637 *tm, uint8_t brightness, uint8_t display_on);

void tm_display_rawN(TM1637 *tm, uint8_t n, const uint8_t *segs);
void tm_display_raw6(TM1637 *tm, const uint8_t logical[6]);

void tm_clear6(TM1637 *tm);

#endif
