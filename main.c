#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <stdint.h>

#define DEBOUNCE_MS 20

#define KEY_ESCAPE  (1 << PB0)
#define KEY_BACK    (1 << PB1)
#define KEY_DOWN    (1 << PB2)
#define KEY_UP      (1 << PB3)
#define KEY_ENTER   (1 << PB4)

#define KEY_MASK    (KEY_ESCAPE | KEY_BACK | KEY_DOWN | KEY_UP | KEY_ENTER)

volatile uint32_t timer_1ms = 0;

volatile uint8_t check_keyboard = 0;

uint8_t keyboard_current = 0xff;
uint8_t keyboard_previous = 0x00;
uint8_t keyboard_press;
uint8_t keyboard_release;

ISR(TIMER1_COMPA_vect) {
    timer_1ms += 1;
    check_keyboard = 1;
}

void step_keyboard() {
    static uint8_t stable = 0xFF;
    static uint8_t age[5] = { 0 };

    uint8_t raw = PINB;

    for (uint8_t i = 0; i < 4; i++) {
        uint8_t mask = (1 << i);
        uint8_t raw_bit = raw & mask;

        if ((stable & mask) == raw_bit) {
            age[i] = 0;
        } else {
            if (++age[i] >= DEBOUNCE_MS) {
                age[i] = 0;

                if (raw_bit) {
                    stable |= mask;
                } else {
                    stable &= ~mask;
                }

                keyboard_current = stable;
            }
        }
    }

    keyboard_press   =  keyboard_current & ~keyboard_previous;
    keyboard_release = ~keyboard_current &  keyboard_previous;

    keyboard_previous = keyboard_current;
}

void reset() {

}

uint8_t aaa = 0;


void start() {
    DDRB |= (1 << PB5);


    DDRB &= ~KEY_MASK;
    PORTB |= KEY_MASK;

    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11) | (1 << CS10);
    OCR1A = 249;
    TIMSK1 |= (1 << OCIE1A);

    sei();
}

void loop() {
    if (check_keyboard) {
        check_keyboard = 0;
        step_keyboard();

        if (keyboard_press & KEY_ESCAPE) {
            aaa = !aaa;
        }
    }

    if (aaa) {
        PORTB |= (1 << PB5);
    } else {
        PORTB &= ~(1 << PB5);
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
