#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "relay.h"
#include "tm.h"
#include "lzpsh.h"
#include "segments.h"
#include "version.h"
#include "keyboard.h"


#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define CLAMP(v,a,b) MIN(MAX(v, a), b)



#define KEY_S1 0b00000001
#define KEY_S2 0b0000010
#define KEY_S3 0b0000100
#define KEY_S4 0b0001000
#define KEY_S5 0b00010000
#define KEY_S6 0b00100000
#define KEY_S7 0b01000000
#define KEY_S8 0b10000000

#define KEY_PRESSED(keyboard, key) ((keyboard & key) == key)


volatile uint8_t check_keyboard = 0;
volatile uint8_t check_reset = 0;
volatile uint8_t check_tick_1s = 0;


int8_t buzz_1 = 0;

IO io_rel_pc3;
Relay rel_1;
IO io_tm_stb;
IO io_tm_clk;
IO io_tm_dio;
TM tm;

void lzpsh_setup(uint8_t array[16]) {
    for (uint8_t i = 0; i < 5; i++) {
        array[i * 2] = lzpsh_7seg[i];
    }

    array[5 * 2] = segment_for_character(' ');
    array[6 * 2] = segment_for_int(2);
    array[7 * 2] = segment_for_int(6);
}

void version_setup(uint8_t array[16]) {
    build_version_segments(APP_VERSION_STRING, array);
}

static volatile uint16_t g_ms = 0;

static void timer1_init_1ms() {
    TCCR1A = 0;
    TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);
    OCR1A  = 249;
    TIMSK1 = (1<<OCIE1A);
}


typedef enum {
    SETUP,
    RUNNING,
    ALARM
} shift_mode_t;

shift_mode_t shift_mode;

ISR(TIMER1_COMPA_vect) {
    static uint16_t sec_cnt = 0;

    g_ms++;
    check_keyboard = 1;

    if (++sec_cnt >= 1000) {
        sec_cnt = 0;
        check_tick_1s = 1;
    }
}


static uint32_t ms_get_atomic() {
    uint32_t t;
    uint8_t s = SREG;
    cli();
    t = g_ms;
    SREG = s;
    return t;
}


keyboard_t keys;

int16_t shift_time = 0;
int16_t shift_left = 0;


int32_t delay_held = 0;


void reset() {
    shift_time = 2 * 60;
    shift_mode = SETUP;
    delay_held = 0;

    relay_off(&rel_1);
}

void start() {
    io_rel_pc3 = io_new_pc3();
    rel_1 = relay_new(&io_rel_pc3);

    io_tm_stb = io_new_pc0();
    io_tm_clk = io_new_pc1();
    io_tm_dio = io_new_pc2();

    tm = tm_new(&io_tm_stb, &io_tm_clk, &io_tm_dio);

    tm_set_brightness(&tm, 7);

    keys = keyboard_new();

//    uint8_t logo[16] = { 0x00 };
//    lzpsh_setup(logo);
//    tm_write_data(&tm, 0, logo, 16);
//    _delay_ms(2000);

    uint8_t version[16] = { 0x00 };
    version_setup(version);
    tm_write_data(&tm, 0, version, 16);
    _delay_ms(2000);

    timer1_init_1ms();
    sei();
}

// MARK: -


typedef struct {
    uint8_t m1;
    uint8_t m2;
    uint8_t s1;
    uint8_t s2;
} display_time_t;

display_time_t convert_to_display_time(uint16_t value) {
    uint16_t mm = value / 60;
    uint16_t ss = value % 60;

    display_time_t t;
    t.m1 = (mm / 10) % 10;
    t.m2 = (mm /  1) % 10;
    t.s1 = (ss / 10) % 10;
    t.s2 = (ss /  1) % 10;
    return t;
}

void display_store_time(display_time_t time, uint8_t buffer[16]) {
    buffer[0]  = segment_for_int(time.m1);
    buffer[2] = segment_for_int(time.m2);
    buffer[4] = segment_for_int(time.s1);
    buffer[6] = segment_for_int(time.s2);
}


void update_display(int16_t value) {
    uint8_t buffer[16] = {0};

    buffer[12] = keys.current;
    buffer[14] = keys.held;

    display_time_t time = convert_to_display_time(value);
    display_store_time(time, buffer);


    buffer[shift_mode * 2 + 1] |= 1;
    buffer[2] |= segment_dot;

    tm_write_data(&tm, 0, buffer, 16);
}

void shift_setup(uint32_t delta_ms) {
    if (delay_held > 0) {
        delay_held = delay_held - delta_ms;
    } else {
        delay_held = 0;

        if (keys.held & KEY_S1) {
            shift_time -= 1 * 60;
            delay_held = 70;
        } else if (keys.held & KEY_S2) {
            shift_time += 1 * 60;
            delay_held = 70;
        } else if (keys.held & KEY_S3) {
            shift_time -= 1;
            delay_held = 70;
        } else if (keys.held & KEY_S4) {
            shift_time += 1;
            delay_held = 70;
        }
    }

    if (keys.press & KEY_S1) {
        shift_time -= 1 * 60;
    } else if (keys.press & KEY_S2) {
        shift_time += 1 * 60;
    } else if (keys.press & KEY_S3) {
        shift_time -= 1;
    } else if (keys.press & KEY_S4) {
        shift_time += 1;
    }

    if (keys.release & KEY_S8) {
        shift_mode = RUNNING;
        shift_left = shift_time;
        check_tick_1s = 0;
    }

    shift_time = CLAMP(shift_time, 0, 10 * 60);
    update_display(shift_time);
}

// MARK: -

static uint32_t last_ms = 0;

void shift_running() {
    if (check_tick_1s) {
        check_tick_1s = 0;

        shift_left -= 1;

        if (shift_left == -1) {
            buzz_1 = 3;
            shift_mode = ALARM;
            relay_on(&rel_1);
        }
    }

    update_display(shift_left);
}


void shift_alarm() {
    if (check_tick_1s) {
        check_tick_1s = 0;

        buzz_1 -= 1;

        if (buzz_1 == -1) {
            relay_off(&rel_1);

            shift_left = shift_time;
            shift_mode = RUNNING;
        }
    }

    update_display(buzz_1);
}

void loop() {
    uint32_t now_ms = ms_get_atomic();
    uint32_t delta_ms = now_ms - last_ms;

    if (delta_ms != 0) {
        last_ms = now_ms;
    }

    if (check_keyboard) {
        check_keyboard = 0;
        keyboard_process(&keys, tm_read_keys(&tm), delta_ms);
    }

    if (check_reset && (keys.release & KEY_S5)) {
        check_reset = 0;
        reset();
    } else if (keys.held & KEY_S5) {
        check_reset = 1;
    }

    switch (shift_mode) {
        case SETUP:
            shift_setup(delta_ms);
            break;
        case RUNNING:
            shift_running();
            break;
        case ALARM:
            shift_alarm();
            break;
    }
}

int main() {
    reset();

    start();

    while (1) {
        loop();
    }
}
