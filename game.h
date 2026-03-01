#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "keyboard.h"

typedef struct {
    uint32_t buzz_time_ms;
    uint32_t shift_time_s;
    uint32_t left_ms;
} game_t;

typedef enum {
    INTRO,
    RUNNING_GAME,
    RUNNING_BUZZ
} game_index_t;

void game_reset();
void game_enter();
void game_tick_100ms();
void game_loop(uint8_t segments[6], keyboard_t keys);

#endif
