#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define TM_DDR   DDRC
#define TM_PORT  PORTC
#define TM_PIN   PINC
#define TM_STB   PC0
#define TM_CLK   PC1
#define TM_DIO   PC2

#define REL_PORT PORTC
#define REL_DDR  DDRC
#define REL_A    PC3

static inline void tm_delay() {
    _delay_us(2);
}

static inline void stb_low() {
    TM_PORT &= ~_BV(TM_STB);
}

static inline void stb_high() {
    TM_PORT |= _BV(TM_STB);
}

static inline void clk_low() {
    TM_PORT &= ~_BV(TM_CLK);
}

static inline void clk_high() {
    TM_PORT |= _BV(TM_CLK);
}

static inline void dio_out() {
    TM_DDR |= _BV(TM_DIO);
}

static inline void dio_in_pullup() {
    TM_DDR &= ~_BV(TM_DIO);
    TM_PORT |= _BV(TM_DIO);
}

static inline void dio_low() {
    dio_out();
    TM_PORT &= ~_BV(TM_DIO);
}

static inline void dio_high() {
    dio_out();
    TM_PORT |= _BV(TM_DIO);
}

static inline uint8_t dio_read() {
    return (TM_PIN & _BV(TM_DIO)) ? 1 : 0;
}

static void tm_bus_init() {
    TM_DDR |= _BV(TM_STB) | _BV(TM_CLK);
    TM_PORT |= _BV(TM_STB) | _BV(TM_CLK);

    dio_in_pullup();
}

static inline void relay_init() {
    REL_DDR |= _BV(REL_A);
    REL_PORT |= _BV(REL_A);
}

static inline void relay_set(uint8_t on) {
    if (on) {
        REL_PORT &= ~_BV(REL_A);
    } else {
        REL_PORT |= _BV(REL_A);
    }
}

static void tm_start() {
    dio_out();
    stb_low();
    tm_delay();
}

static void tm_stop() {
    stb_high();
    tm_delay();
    dio_in_pullup();
}

static void tm_write_byte(uint8_t b) {
    dio_out();

    for (uint8_t i = 0; i < 8; i++) {
        clk_low();
        tm_delay();

        if (b & 0x01) {
            dio_high();
        } else {
            dio_low();
        }

        tm_delay();
        clk_high();
        tm_delay();

        b >>= 1;
    }
}

static uint8_t tm_read_byte() {
    uint8_t b = 0;
    dio_in_pullup();

    for (uint8_t i = 0; i < 8; i++) {
        clk_low();
        tm_delay();
        clk_high();
        tm_delay();
        if (dio_read()) {
            b |= (1 << i);
        }
    }

    return b;
}

static void tm_cmd(uint8_t c) {
    tm_start();
    tm_write_byte(c);
    tm_stop();
}

static void tm_set_brightness(uint8_t b) {
    if (b > 7) {
        b = 7;
    }

    tm_cmd(0x88 | b);
}

static void tm_write_data_auto(uint8_t addr, const uint8_t *data, uint8_t len) {
    tm_cmd(0x40);
    tm_start();
    tm_write_byte(0xC0 | (addr & 0x0F));

    for (uint8_t i = 0; i < len; i++) {
        tm_write_byte(data[i]);
    }

    tm_stop();
}

static const uint8_t seg_digit[10] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

static void tm_clear(void) {
    uint8_t zeros[16] = {0};
    tm_write_data_auto(0, zeros, 16);
}

static uint8_t tm_read_keys() {
    uint8_t keys = 0;
    tm_start();
    tm_write_byte(0x42);

    for (uint8_t i = 0; i < 4; i++) {
        uint8_t v = tm_read_byte();
        keys |= (v << i);
    }

    tm_stop();
    return keys;
}

int main(void) {
    tm_bus_init();
    relay_init();

    tm_set_brightness(7);
    tm_clear();

    while (1) {
        _delay_ms(50);

        uint8_t k = tm_read_keys();

        uint8_t buf[16] = {0};

        buf[0] = k & 0x0F;
        buf[2] = (k >> 4) & 0x0F;

        for (uint8_t i=0; i<8; i++) {
            buf[i * 2 + 1] = (k & ( 1 << i)) ? 1 : 0;
        }

        tm_write_data_auto(0, buf, 16);

        relay_set(k != 0);
    }
}
