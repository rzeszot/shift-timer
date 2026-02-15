#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include "io.h"
#include "button.h"
#include "tm1637.h"
#include "relay.h"
#include "config.h"
#include "keyboard.h"

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

#define DEBOUNCE_MS 200

#define KEY_ESCAPE  (1 << 0)
#define KEY_BACK    (1 << 1)
#define KEY_DOWN    (1 << 2)
#define KEY_UP      (1 << 3)
#define KEY_ENTER   (1 << 4)

keyboard_t keyboard;


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

config_t config;




volatile uint32_t timer_1ms = 0;
volatile uint32_t timer_1s = 0;

volatile uint8_t keyboard_check = 0;
volatile uint8_t check_tm = 0;


ISR(TIMER1_COMPA_vect) {
    timer_1ms += 1;

    keyboard_check = 1;

//    if (timer_1ms % 20 == 0) {
//        check_keyboard = 1;
//    }
//
//    if (timer_1ms % 500 == 0) {
//        check_tm = 1;
//    }
//
//    if (timer_1ms >= 1000) {
//        timer_1ms = 0;
//        timer_1s += 1;
//    }
}
//
//
//uint8_t xxx = 0;
//
//void keyboard_run() {
//    uint8_t keys = 0
//        | (button_read(&button_pb0) << 0)
//        | (button_read(&button_pb1) << 1)
//        | (button_read(&button_pb2) << 2)
//        | (button_read(&button_pb3) << 3)
//        | (button_read(&button_pb4) << 4);
//    keys ^= 0b00011111;
//
//    keyboard_process(&keyboard, keys);
//
//    if (keyboard.press & KEY_UP) {
//        xxx += 1;
//    } else if (keyboard.press & KEY_DOWN) {
//        xxx -= 1;
//    }
//}
//
//void tm_run() {
//    uint8_t segs[6] = { 0 };
//
//    segs[0] = keyboard.current;
//    segs[4] = DIGIT_SEG[xxx / 10];
//    segs[5] = DIGIT_SEG[xxx % 10];
//
//    tm_display_raw6(&tm, segs);
//}
//
//void reset() {
//
//}

void start() {
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11) | (1 << CS10);
    OCR1A = 249;
    TIMSK1 |= (1 << OCIE1A);

    sei();

    keyboard = keyboard_new();

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

    button_pb0 = button_new(&io_button_pb0);
    button_pb1 = button_new(&io_button_pb1);
    button_pb2 = button_new(&io_button_pb2);
    button_pb3 = button_new(&io_button_pb3);
    button_pb4 = button_new(&io_button_pb4);

    tm_set_brightness(&tm, 0x07, 1);
    tm_clear6(&tm);
}

//void loop() {
//    if (keyboard_check) {
//        keyboard_check = 0;
//        keyboard_run();
//    }
//
//    if (check_tm) {
//        check_tm = 0;
//        tm_run();
//    }
//}

uint8_t aaa = 0;
static uint8_t stable = 1;
static uint8_t ticks = 0;
static uint8_t g_key_level = 0;

int main() {
    start();

    DDRB |= (1 << PB5);

    DDRB &= ~(1 << PB0);
    PORTB |= (1 << PB0);


    while (1) {
        if (keyboard_check) {
            keyboard_check = 0;

            uint8_t raw = (PINB & (1 << PB0)) ? 1 : 0;

//            if (raw == stable) {
//                ticks = 0;
//            } else if (++ticks >= DEBOUNCE_MS) {
//                stable = raw;
//                ticks = 0;
//                g_key_level = stable;
//            }


        }




        if (!g_key_level) {
            aaa = !aaa;
        }

        if (aaa) {
            PORTB |= (1 << PB5);
        } else {
            PORTB &= ~(1 << PB5);
        }
    }

    return 0;
}
