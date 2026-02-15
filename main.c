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

#define TM_DIO       PC0
#define TM_CLK       PC1

volatile uint32_t timer_1ms = 0;
volatile uint8_t tm_check = 0;
volatile uint8_t keyboard_check = 0;
uint8_t keyboard_press;
uint8_t keyboard_release;

uint8_t segments[6];

typedef enum {
    NONE,
    HIGH,
    LOW
} tm_state_t;

typedef struct {
    tm_state_t clk;
    tm_state_t dio;
} tm_io_t;

#define TMB_SIZE 64

typedef struct {
    tm_io_t buffer[TMB_SIZE];
    uint8_t head;
    uint8_t tail;
} tm_buffer_t;


tm_buffer_t buffer;

void tmb_init(tm_buffer_t *b) {
    b->head = 0;
    b->tail = 0;
}

bool tmb_push(tm_buffer_t *b, tm_io_t data) {
    uint8_t next = (b->head + 1) % TMB_SIZE;

    if (next == b->tail) {
        return false;
    }

    b->buffer[b->head] = data;
    b->head = next;
    return true;
}

bool tmb_is_empty(tm_buffer_t *b) {
    return b->head == b->tail;
}

bool tmb_pop(tm_buffer_t *b, tm_io_t *data) {
    if (tmb_is_empty(b)) {
        return false;
    }

    *data = b->buffer[b->tail];
    b->tail = (b->tail + 1) % TMB_SIZE;

    return true;
}

void tmb_push_start(tm_buffer_t *b) {
    tmb_push(b, (tm_io_t){ .clk = HIGH, .dio = HIGH });
    tmb_push(b, (tm_io_t){ .clk = HIGH, .dio =  LOW });
    tmb_push(b, (tm_io_t){ .clk =  LOW, .dio =  LOW });
}

void tmb_push_stop(tm_buffer_t *b) {
    tmb_push(b, (tm_io_t){ .clk =  LOW, .dio =  LOW });
    tmb_push(b, (tm_io_t){ .clk = HIGH, .dio =  LOW });
    tmb_push(b, (tm_io_t){ .clk = HIGH, .dio = HIGH });
}

//void tmb_push_byte(tm_buffer_t *b, uint8_t byte) {
//    for (uint8_t i = 0; i < 8; i++) {
//        TM_CLK_L();
//
//        tm_delay();
//
//        if (byte & 1) {
//            TM_DIO_H();
//        } else {
//            TM_DIO_L();
//        }
//
//
//        tm_delay();
//
//        TM_CLK_H();
//
//        tm_delay();
//
//        byte >>= 1;
//    }
//
//
//
//
//
//
//    tmb_push(b, (tm_io_t){ .clk = 1, .dio = 1 });
//}



#define TM_DIO PC0
#define TM_CLK PC1

#define TM_DIO_OUT()   (DDRC |=  (1U << TM_DIO))
#define TM_DIO_IN()    (DDRC &= ~(1U << TM_DIO))
#define TM_DIO_H()     (PORTC |=  (1U << TM_DIO))
#define TM_DIO_L()     (PORTC &= ~(1U << TM_DIO))
#define TM_DIO_READ()  ((PINC >> TM_DIO) & 1U)

#define TM_CLK_OUT()   (DDRC |=  (1U << TM_CLK))
#define TM_CLK_H()     (PORTC |=  (1U << TM_CLK))
#define TM_CLK_L()     (PORTC &= ~(1U << TM_CLK))

static inline void tm_delay(void) { _delay_us(20); }


static void tm_write_byte(uint8_t b)
{

    
    for (uint8_t i = 0; i < 8; i++)
    {
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
    TM_DIO_L();

    tm_delay();

    TM_CLK_L();
    TM_DIO_L();

    tm_delay();
}

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

    if (timer_1ms % 20 == 0) {
        tm_check = 1;
    }
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

void tm_step() {

    const uint8_t digits[] = {
        0x4F,  // 3
        0,  // 2
        segment_for_int(aaa),  // 1
        0x7d,  // 6
        0x6d,  // 5
        0x66   // 4
    };


    // start

    TM_CLK_H();
    TM_DIO_H();

    tm_delay();

    TM_CLK_H();
    TM_DIO_L();

    tm_delay();

    TM_CLK_L();
    TM_DIO_L();

    tm_delay();

    // ...

    tm_write_byte(0x40);

    // stop

    TM_CLK_L();
    TM_DIO_L();

    tm_delay();

    TM_CLK_H();
    TM_DIO_L();

    tm_delay();

    TM_CLK_H();
    TM_DIO_H();

    tm_delay();

    // start

    TM_CLK_H();
    TM_DIO_H();

    tm_delay();

    TM_CLK_H();
    TM_DIO_L();

    tm_delay();

    TM_CLK_L();
    TM_DIO_L();

    tm_delay();

    // ..

    tm_write_byte(0xC0);


    for (uint8_t i = 0; i < 6; i++) {
        tm_write_byte(digits[i]);
    }

    // stop

    TM_CLK_L();
    TM_DIO_L();

    tm_delay();

    TM_CLK_H();
    TM_DIO_L();

    tm_delay();

    TM_CLK_H();
    TM_DIO_H();

    tm_delay();
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

    TM_DIO_OUT();
    TM_DIO_H();

    TM_CLK_OUT();
    TM_CLK_H();

    led_set(0);
    buzz_set(0);

    for(uint8_t i=0; i<6; i++) {
        segments[i] = i;
    }


    TM_CLK_H();
    TM_DIO_H();

    tm_delay();

    TM_DIO_L();

    tm_delay();

    TM_CLK_L();

    tm_delay();


    // display on + jasność max

    tm_write_byte(0x80 | 0x08 | 0x07);


    // stop

    TM_CLK_L();
    TM_DIO_L();

    tm_delay();

    TM_CLK_H();

    tm_delay();

    TM_DIO_H();

    tm_delay();
}


void loop() {
    if (keyboard_check) {
        keyboard_check = 0;
        keyboard_step();

        if (keyboard_press & KEY_ESCAPE) {
            aaa -= 1;
        }
        if (keyboard_release & KEY_BACK) {
            aaa += 1;
        }
    }

    if (tm_check) {
        tm_check = 0;
        tm_step();
    }

//    buzz_set(aaa);
//    led_set(bbb);
}

int main() {
    start();

    while (1) {
        loop();
    }

    return 0;
}
