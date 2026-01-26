#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include "relay.h"
#include "tm.h"

#define TM_DDR   DDRC
#define TM_PORT  PORTC
#define TM_PIN   PINC
#define TM_STB   PC0
#define TM_CLK   PC1
#define TM_DIO   PC2

IO io_rel_pc3;
Relay rel_1;

IO io_tm_stb;
IO io_tm_clk;
IO io_tm_dio;

TM tm;

static const uint8_t seg_digit[10] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

static volatile uint16_t g_ms = 0;
static volatile uint8_t g_tick_1s = 0;

static void timer1_init_1ms() {
    TCCR1A = 0;
    TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);
    OCR1A  = 249;
    TIMSK1 = (1<<OCIE1A);
}

ISR(TIMER1_COMPA_vect) {
    if (++g_ms >= 1000){
        g_ms = 0;
        g_tick_1s = 1;
    }
}

uint16_t total = 90;
uint16_t left = 90;
uint8_t blink = 0;

struct time_hms_t {
    uint8_t h1;
    uint8_t h2;
    uint8_t m1;
    uint8_t m2;
    uint8_t s1;
    uint8_t s2;
};

static struct time_hms_t convert_to_time(uint16_t left) {
    uint16_t hh = left / 3600;
    uint16_t rem = left % 3600;
    uint16_t mm = rem / 60;
    uint16_t ss = rem % 60;

    struct time_hms_t t;
    t.h1 = (hh / 10) % 10;
    t.h2 = (hh /  1) % 10;
    t.m1 = (mm / 10) % 10;
    t.m2 = (mm /  1) % 10;
    t.s1 = (ss / 10) % 10;
    t.s2 = (ss /  1) % 10;
    return t;
}

void start() {
    io_rel_pc3 = io_new_pc3();
    rel_1 = relay_new(&io_rel_pc3);

    io_tm_stb = io_new_pc0();
    io_tm_clk = io_new_pc1();
    io_tm_dio = io_new_pc2();

    tm = tm_new(&io_tm_stb, &io_tm_clk, &io_tm_dio);

    tm_set_brightness(&tm, 7);

    uint8_t logo[16] = {
        0x38, // L
        0x00,
        0x5B, // 2
        0x00,
        0x73, // P
        0x00,
        0x6D, // S
        0x00,
        0x76, // H
        0x00,
        0x00,
        0x00,
        0x5b, // 2
        0x00,
        0x7d, // 6
        0x00
    };
    tm_write_data(&tm, 0, logo, 16);

    _delay_ms(2000);

    uint8_t version[16] = {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x3f | 0x80, // 0.
        0x00,
        0x3f | 0x80, // 0.
        0x00,
        0x06, // 1
        0x00
    };
    tm_write_data(&tm, 0, version, 16);

    _delay_ms(1000);
}

void loop() {

}

int main() {
    start();

    timer1_init_1ms();
    sei();
    

    while (1) {
        loop();
        _delay_ms(50);

        uint8_t k = tm_read_keys(&tm);

        uint8_t buf[16] = {0};

        buf[0] = k & 0x0F;
        buf[2] = (k >> 4) & 0x0F;

        if (g_tick_1s){
            g_tick_1s = 0;
            left -= 1;

            blink ^= 1;
        }

        struct time_hms_t time = convert_to_time(left);

        buf[4] = seg_digit[time.h1];
        buf[6] = seg_digit[time.h2];
        buf[8]  = seg_digit[time.m1];
        buf[10] = seg_digit[time.m2];
        buf[12] = seg_digit[time.s1];
        buf[14] = seg_digit[time.s2];
        if (blink) {
            buf[6] |= 0x80;
        } else {
            buf[10] |= 0x80;
        }

        tm_write_data(&tm, 0, buf, 16);

        relay_set(&rel_1, k != 0);
    }
}
