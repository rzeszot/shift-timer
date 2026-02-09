#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdint.h>

#include "io.h"
#include "button.h"
#include "tm1637.h"
#include "relay.h"
#include "config.h"

IO io_tm_dio;
IO io_tm_clk;
TM1637 tm;

IO io_rel_pc4;
IO io_rel_pc0;
Relay rel_1;
Relay rel_2;

IO io_button_pb0;
IO io_button_pb1;
IO io_button_pb2;
IO io_button_pb3;
IO io_button_pb4;

Button button_pb0;
Button button_pb1;
Button button_pb2;
Button button_pb3;
Button button_pb4;

volatile uint32_t timer_1ms = 0;
volatile uint32_t timer_1s = 0;

ISR(TIMER1_COMPA_vect) {
    timer_1ms += 1;

    if (timer_1ms >= 1000) {
        timer_1ms = 0;
        timer_1s += 1;
    }
}


volatile uint8_t keys = 0b00000000;

static const uint8_t DIGIT_SEG[10] = {
    0x3F, /* 0 */
    0x06, /* 1 */
    0x5B, /* 2 */
    0x4F, /* 3 */
    0x66, /* 4 */
    0x6D, /* 5 */
    0x7D, /* 6 */
    0x07, /* 7 */
    0x7F, /* 8 */
    0x6F  /* 9 */
};

static void tm_display_hhmmss(uint8_t hh, uint8_t mm, uint8_t ss) {
    uint8_t segs[6];

    segs[0] = DIGIT_SEG[(hh / 10) % 10];
    segs[1] = (uint8_t)(DIGIT_SEG[hh % 10] | 0x80);  /* colon after HH */
    segs[2] = DIGIT_SEG[(mm / 10) % 10];
    segs[3] = (uint8_t)(DIGIT_SEG[mm % 10] | 0x80);  /* colon after MM */
    segs[4] = DIGIT_SEG[(ss / 10) % 10];
    segs[5] = DIGIT_SEG[ss % 10];

    tm_display_raw6(&tm, segs);
}


config_t config;

void reset() {

}

void start() {
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11) | (1 << CS10);
    OCR1A = 249;
    TIMSK1 |= (1 << OCIE1A);

    sei();

    io_tm_dio = io_new_pd2();
    io_tm_clk = io_new_pd3();
    tm = tm1637_new(&io_tm_clk, &io_tm_dio);

    io_rel_pc0 = io_new_pc0();
    rel_1 = relay_new(&io_rel_pc0);
    io_rel_pc4 = io_new_pc4();
    rel_2 = relay_new(&io_rel_pc4);

    io_button_pb0 = io_new_pb0();
    io_button_pb1 = io_new_pb1();
    io_button_pb2 = io_new_pb2();
    io_button_pb3 = io_new_pb3();
    io_button_pb4 = io_new_pb4();

    io_in(&io_button_pb0);
    io_in(&io_button_pb1);
    io_in(&io_button_pb2);
    io_in(&io_button_pb3);
    io_in(&io_button_pb4);


    io_high(&io_button_pb0);
    io_high(&io_button_pb1);
    io_high(&io_button_pb2);
    io_high(&io_button_pb3);
    io_high(&io_button_pb4);

    button_pb0 = button_new(&io_button_pb0);
    button_pb1 = button_new(&io_button_pb1);
    button_pb2 = button_new(&io_button_pb2);
    button_pb3 = button_new(&io_button_pb3);
    button_pb4 = button_new(&io_button_pb4);

    tm_set_brightness(&tm, 0x07, 1);
    tm_clear6(&tm);
}

void loop() {
    uint8_t keys = 0
        | (button_read(&button_pb0) << 0)
        | (button_read(&button_pb1) << 1)
        | (button_read(&button_pb2) << 2)
        | (button_read(&button_pb3) << 3)
        | (button_read(&button_pb4) << 4);
    keys ^= 0b00011111;

    tm_display_hhmmss(keys, timer_1ms / 100, timer_1ms % 100);
}

int main() {
    reset();
    start();

    while(1) {
        loop();
    }

    return 0;
}
