#include <avr/eeprom.h>
#include "config.h"

config_t EEMEM config_eeprom;

const config_t config_default = {
    .version      = 2,
    .buzz_time_ms = 3000,
    .shift_time_s = 120
};

config_t config_read() {
    config_t r;

    eeprom_read_block(&r, &config_eeprom, sizeof(config_t));

    if (r.version != config_default.version) {
        r = config_default;
        eeprom_update_block(&r, &config_eeprom, sizeof(config_t));
    }

    return r;
}

void config_save(config_t c) {
    c.version = config_default.version;
    eeprom_update_block(&c, &config_eeprom, sizeof(config_t));
}
