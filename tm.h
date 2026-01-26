#ifndef TM_H
#define TM_H

#include "io.h"

typedef struct {
    IO stb;
    IO clk;
    IO dio;
} TM;

TM tm_new(IO *stb, IO *clk, IO *dio);

void tm_start(TM *tm);
void tm_stop(TM *tm);

void tm_write_byte(TM *tm, uint8_t b);
void tm_write_data(TM *tm, uint8_t addr, const uint8_t *data, uint8_t len);

uint8_t tm_read_byte(TM *tm);
uint8_t tm_read_keys(TM *tm);

void tm_cmd(TM *tm, uint8_t c);

void tm_set_brightness(TM *tm, uint8_t b);

void tm_clear(TM *tm);

void tm_delay();

#endif
