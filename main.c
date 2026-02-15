#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <stdint.h>

#define KEY_DEBOUNCE_MS   20
#define KEY_LONG_PRESS_MS 500

#define KEY_ESCAPE  (1 << PB0)
#define KEY_BACK    (1 << PB1)
#define KEY_DOWN    (1 << PB2)
#define KEY_UP      (1 << PB3)
#define KEY_ENTER   (1 << PB4)

volatile uint32_t timer_1ms = 0;
volatile uint8_t keyboard_check = 0;
uint8_t keyboard_press;
uint8_t keyboard_release;

uint8_t segments[6];

void buzz_set(uint8_t enabled) {
    DDRC |= (1 << PC5);

    if (enabled) {
        PORTC &= ~(1 << PC5);
    } else {
        PORTC |= (1 << PC5);
    }
}

void led_set(uint8_t enabled) {
    DDRB |= (1 << PB5);

    if (enabled) {
        PORTB |= (1 << PB5);
    } else {
        PORTB &= ~(1 << PB5);
    }
}

ISR(TIMER1_COMPA_vect) {
    timer_1ms += 1;
    keyboard_check = 1;
}

void keyboard_debounce() {
    static uint8_t stable = 0x00;
    static uint8_t age[5] = { 0 };

    static uint8_t current = 0x00;
    static uint8_t previous = 0x00;

    uint8_t raw = PINB;

    for (uint8_t i = 0; i < 4; i++) {
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

    led_set(0);
    buzz_set(0);

    for(uint8_t i=0; i<6; i++) {
        segments[i] = i;
    }
}

uint8_t aaa = 0;
uint8_t bbb = 0;

void loop() {
    if (keyboard_check) {
        keyboard_check = 0;

        keyboard_debounce();

        if (keyboard_press & KEY_ESCAPE) {
            aaa = !aaa;
        }

        if (keyboard_release & KEY_BACK) {
            bbb = !bbb;
        }
    }

    buzz_set(aaa);
    led_set(bbb);
}

int main() {
    start();

    while (1) {
        loop();
    }

    return 0;
}
