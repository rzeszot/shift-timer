#ifndef EDITOR_H
#define EDITOR_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "keyboard.h"

typedef enum {
    BUZZ_TIME,
    SHIFT_TIME
} editor_index_t;

extern editor_index_t editor_index;
extern bool editor_edit;

extern config_t editor_config;

void editor_reset();
void editor_loop(uint8_t segments[6], keyboard_t keys);


#endif
