#include "game.h"
#include "segments.h"
#include "config.h"
#include "buzz.h"

typedef struct {
    uint8_t minutes;
    uint8_t seconds;
    uint8_t tenths;
} display_t;

display_t time_ms_to_display(uint32_t time) {
    display_t result;

    time /= 100;

    result.minutes = time / 600;
    result.seconds = (time / 10) % 60;
    result.tenths = time % 10;

    return result;
}

game_t game_state;
game_index_t game_index = INTRO;

void game_reset() {

}

void game_enter() {
    config_t config = config_read();

    game_state.buzz_time_ms = config.buzz_time_ms;
    game_state.shift_time_s = config.shift_time_s;

    game_index = INTRO;

    buzz_set(0);
}


void game_tick_100ms() {
    switch (game_index) {
        case INTRO:
            break;
        case RUNNING_GAME:
            game_state.left_ms -= 100;

            if (game_state.left_ms == 0) {
                game_state.left_ms = game_state.buzz_time_ms;
                game_index = RUNNING_BUZZ;
            }
            break;
        case RUNNING_BUZZ:
            game_state.left_ms -= 100;
            if (game_state.left_ms == 0) {
                game_state.left_ms = game_state.shift_time_s * 1000;
                game_index = RUNNING_GAME;
            }
            break;
    }
}

void game_loop(uint8_t segments[6], keyboard_t keys) {
    for (uint8_t i=0; i<6; i++) {
        segments[i] = 0;
    }

    display_t time = {
        .minutes = 0,
        .seconds = 0,
        .tenths = 0
    };

    switch (game_index) {
        case INTRO:
            time = time_ms_to_display(game_state.shift_time_s * 1000);
            if (keys.press & KEY_ENTER) {
                game_state.left_ms = game_state.shift_time_s * 1000;
                game_index = RUNNING_GAME;
            }
            break;
        case RUNNING_GAME:
            buzz_set(0);
            time = time_ms_to_display(game_state.left_ms);
            break;
        case RUNNING_BUZZ:
            buzz_set(1);
            time = time_ms_to_display(game_state.left_ms);
            break;
    }

    switch (game_index) {
        case INTRO:
            segments[1] = segment_for_int(time.minutes / 10);
            segments[2] = segment_for_int(time.minutes % 10) | 0x80;
            segments[3] = segment_for_int(time.seconds / 10);
            segments[4] = segment_for_int(time.seconds % 10) | 0x80;
            segments[5] = segment_for_int(time.tenths);
            break;

        case RUNNING_GAME:
            segments[1] = segment_for_int(time.minutes / 10);
            segments[2] = segment_for_int(time.minutes % 10) | 0x80;
            segments[3] = segment_for_int(time.seconds / 10);
            segments[4] = segment_for_int(time.seconds % 10) | 0x80;
            segments[5] = segment_for_int(time.tenths);
            break;
        case RUNNING_BUZZ:
            segments[1] = segment_for_int(time.minutes / 10);
            segments[2] = segment_for_int(time.minutes % 10) | 0x80;
            segments[3] = segment_for_int(time.seconds / 10);
            segments[4] = segment_for_int(time.seconds % 10) | 0x80;
            segments[5] = segment_for_int(time.tenths);
            break;
    }
}
