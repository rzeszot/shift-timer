#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct {
    uint8_t version;
    uint32_t buzz_time_ms;
    uint32_t shift_time_s;
} config_t;

extern config_t config_eeprom;
extern const config_t config_default;

config_t config_read();
void config_save(config_t c);

#endif
