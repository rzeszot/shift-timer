#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include "segments.h"
#include "config.h"
#include "keyboard.h"

#define KEY_DEBOUNCE_MS   20
#define KEY_LONG_PRESS_MS 500

#define KEY_ESCAPE  (1 << PB0)
#define KEY_BACK    (1 << PB1)
#define KEY_DOWN    (1 << PB2)
#define KEY_UP      (1 << PB3)
#define KEY_ENTER   (1 << PB4)

// MARK: -

#define CONCAT(a, b)     a##b
#define DDR_REG(x)       CONCAT(DDR, x)
#define PORT_REG(x)      CONCAT(PORT, x)
#define PIN_REG(x)       CONCAT(PIN, x)

// MARK: -

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(x,l,h) (MIN(MAX((x),(l)),(h)))

// MARK: -

#define TM_DIO       PC0
#define TM_CLK       PC1

#define TM_DIO_OUT()   (DDRC |=  (1U << TM_DIO))
#define TM_DIO_IN()    (DDRC &= ~(1U << TM_DIO))
#define TM_DIO_H()     (PORTC |=  (1U << TM_DIO))
#define TM_DIO_L()     (PORTC &= ~(1U << TM_DIO))
#define TM_DIO_READ()  ((PINC >> TM_DIO) & 1U)

#define TM_CLK_OUT()   (DDRC |=  (1U << TM_CLK))
#define TM_CLK_H()     (PORTC |=  (1U << TM_CLK))
#define TM_CLK_L()     (PORTC &= ~(1U << TM_CLK))

static inline void tm_delay() {
    _delay_us(10);
}

void tm_command_start() {
    TM_CLK_H();
    TM_DIO_H();

    tm_delay();

    TM_DIO_L();

    tm_delay();

    TM_CLK_L();

    tm_delay();
}

void tm_command_stop() {
    TM_CLK_L();
    TM_DIO_L();

    tm_delay();

    TM_CLK_H();

    tm_delay();

    TM_DIO_H();

    tm_delay();
}

void tm_command_write(uint8_t b) {
    for (uint8_t i = 0; i < 8; i++) {
        TM_CLK_L();

        tm_delay();

        if (b & 1) {
            TM_DIO_H();
        } else {
            TM_DIO_L();
        }

        tm_delay();

        TM_CLK_H();

        tm_delay();

        b >>= 1;
    }

    TM_CLK_L();
    TM_DIO_L();

    tm_delay();

    TM_CLK_H();

    tm_delay();

    TM_CLK_L();

    tm_delay();
}

// MARK: -

uint8_t segments[6];

void segments_init() {
    TM_DIO_OUT();
    TM_DIO_H();

    TM_CLK_OUT();
    TM_CLK_H();

    tm_command_start();
    tm_command_write(0x80 | 0x08 | 0x07);
    tm_command_stop();
}

void segments_update() {
    static uint8_t map[] = { 2, 1, 0, 5, 4, 3 };

    tm_command_start();
    tm_command_write(0x40);
    tm_command_stop();

    tm_command_start();
    tm_command_write(0xC0);
    for (uint8_t i = 0; i < 6; i++) {
        tm_command_write(segments[map[i]]);
    }
    tm_command_stop();
}

// MARK: -

void buzz_set(uint8_t enabled) {
    DDRC |= (1 << PC5);

    if (enabled) {
        PORTC &= ~(1 << PC5);
    } else {
        PORTC |= (1 << PC5);
    }
}


// MARK: -

volatile uint32_t timer_1ms = 0;
volatile uint32_t timer_1s = 0;
volatile uint8_t keyboard_check = 0;

keyboard_t keys;


ISR(TIMER1_COMPA_vect) {
    timer_1ms += 1;
    keyboard_check = 1;

    if (timer_1ms >= 1000) {
        timer_1ms = 0;
        timer_1s += 1;
    }
}

void keyboard_step() {
    static uint8_t stable = 0x00;
    static uint8_t age[5] = { 0 };

    uint8_t raw = PINB;
    uint8_t current = keys.current;

    for (uint8_t i = 0; i < 5; i++) {
        uint8_t mask = (1 << i);
        uint8_t raw_bit = raw & mask;

        if ((stable & mask) == raw_bit) {
            age[i] = 0;
        } else {
            if (++age[i] >= KEY_DEBOUNCE_MS) {
                age[i] = 0;

                if (raw_bit) {
                    stable |= mask;
                } else {
                    stable &= ~mask;
                }

                current = stable ^ 0b00011111;
            }
        }
    }

    keyboard_process(&keys, current);
}



typedef enum {
    BUZZ_TIME,
    SHIFT_TIME
} config_index_t;

config_t config;
config_index_t index;
bool edit;

void config_reset() {
    index = BUZZ_TIME;
    config = config_read();
    edit = false;
}

void config_loop() {
    for (uint8_t i=0; i<6; i++) {
        segments[i] = 0;
    }

    if (edit) {
        if (keys.press & KEY_ENTER) {
            edit = false;
            config_save(config);
        } else if (keys.press & KEY_BACK) {
            edit = false;
            config = config_read();
        } else {
            switch (index) {
                case BUZZ_TIME:
                    if (keys.held & KEY_UP) {
                        config.buzz_time_ms += 1;
                    } else if (keys.held & KEY_DOWN) {
                        config.buzz_time_ms -= 1;
                    } else if (keys.press & KEY_DOWN) {
                        config.buzz_time_ms -= 1;
                    } else if (keys.press & KEY_UP) {
                        config.buzz_time_ms += 1;
                    }
                    config.buzz_time_ms = CLAMP(config.buzz_time_ms, 100, 5 * 1000); // 100ms - 5s
                    break;
                case SHIFT_TIME:
                    if (keys.held & KEY_UP) {
                        config.shift_time_s += 1;
                    } else if (keys.held & KEY_DOWN) {
                        config.shift_time_s -= 1;
                    } else if (keys.press & KEY_DOWN) {
                        config.shift_time_s -= 1;
                    } else if (keys.press & KEY_UP) {
                        config.shift_time_s += 1;
                    }
                    config.shift_time_s = CLAMP(config.shift_time_s, 10, 20 * 60); // 10s - 20m
                    break;
            }
        }
    } else {
        if (keys.press & KEY_UP) {
            if (index == SHIFT_TIME) {
                index = BUZZ_TIME;
            } else {
                index += 1;
            }
        } else if (keys.press & KEY_DOWN) {
            if (index == 0) {
                index = SHIFT_TIME;
            } else {
                index -= 1;
            }
        } else if (keys.press & KEY_ENTER) {
            edit = true;
        }
    }

    segments[0] = segment_for_int(index);

    if (edit) {
        if (timer_1s % 2) {
            segments[0] |= 0x80;
        }
    } else {
        segments[0] |= 0x80;
    }

    switch (index) {
        case BUZZ_TIME:
            segments[2] = segment_for_int(config.buzz_time_ms / 1000 % 10);
            segments[3] = segment_for_int(config.buzz_time_ms / 100 % 10);
            segments[4] = segment_for_int(config.buzz_time_ms / 10 % 10);
            segments[5] = segment_for_int(config.buzz_time_ms / 1 % 10);
            break;
        case SHIFT_TIME:
            segments[2] = segment_for_int(config.shift_time_s / 1000 % 10);
            segments[3] = segment_for_int(config.shift_time_s / 100 % 10);
            segments[4] = segment_for_int(config.shift_time_s / 10 % 10);
            segments[5] = segment_for_int(config.shift_time_s / 1 % 10);
            break;
    }

    segments_update();
}



void reset() {
    buzz_set(0);

    for (uint8_t i=0; i<6; i++) {
        segments[i] = 0;
    }

    config_reset();
    keys = keyboard_new();
}

void start() {
    DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4));
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4);

    PORTC |= (1 << PC5);

    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11) | (1 << CS10);
    OCR1A = 249;
    TIMSK1 |= (1 << OCIE1A);

    sei();

    segments_init();
    segments_update();
}


void loop() {
    if (keyboard_check) {
        keyboard_check = 0;
        keyboard_step();
    }


    config_loop();
}

int main() {
    reset();
    start();

    while (1) {
        loop();
    }

    return 0;
}
