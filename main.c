#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include "segments.h"

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
volatile uint8_t keyboard_check = 0;
uint8_t keyboard_press;
uint8_t keyboard_release;





ISR(TIMER1_COMPA_vect) {
    timer_1ms += 1;
    keyboard_check = 1;
}

void keyboard_step() {
    static uint8_t stable = 0x00;
    static uint8_t age[5] = { 0 };

    static uint8_t current = 0x00;
    static uint8_t previous = 0x00;

    uint8_t raw = PINB;

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

    keyboard_press   =  current & ~previous;
    keyboard_release = ~current &  previous;

    previous = current;
}

uint8_t aaa = 0;




void reset() {
    buzz_set(0);

    for (uint8_t i=0; i<6; i++) {
        segments[i] = 0;
    }
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

        uint8_t qqq = aaa;

        if (keyboard_press & KEY_DOWN) {
            aaa -= 1;
        }
        if (keyboard_release & KEY_UP) {
            aaa += 1;
        }

        if (aaa != qqq) {
            segments[5] = segment_for_int(aaa);
            segments_update();
        }
    }
}

int main() {
    reset();
    start();

    while (1) {
        loop();
    }

    return 0;
}
