#include "editor.h"
#include "segments.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(x,l,h) (MIN(MAX((x),(l)),(h)))

editor_index_t editor_index;
bool editor_edit;
config_t editor_config;

void editor_reset() {
    editor_index = BUZZ_TIME;
    editor_edit = false;

    editor_config = config_read();
}


editor_index_t editor_index_next(editor_index_t value) {
    return (value + 1) % (PLACEHOLDER + 1);
}

editor_index_t editor_index_previous(editor_index_t value) {
    if (value == BUZZ_TIME) {
        return PLACEHOLDER;
    } else {
        return value - 1;
    }
}

void editor_loop(uint8_t segments[6], keyboard_t keys) {
    for (uint8_t i=0; i<6; i++) {
        segments[i] = 0;
    }

    if (editor_edit) {
        if (keys.press & KEY_ENTER) {
            editor_edit = false;
            config_save(editor_config);
        } else if (keys.press & KEY_BACK) {
            editor_edit = false;
            editor_config = config_read();
        } else {
            switch (editor_index) {
                case BUZZ_TIME:
                    if (keys.held & KEY_UP || keys.press & KEY_UP) {
                        editor_config.buzz_time_ms += 10;
                    } else if (keys.held & KEY_DOWN || keys.press & KEY_DOWN) {
                        editor_config.buzz_time_ms -= 10;
                    }
                    editor_config.buzz_time_ms = CLAMP(editor_config.buzz_time_ms, 10, 15 * 1000); // 10ms - 15s
                    break;
                case SHIFT_TIME:
                    if (keys.held & KEY_UP) {
                        editor_config.shift_time_s += 1;
                    } else if (keys.held & KEY_DOWN) {
                        editor_config.shift_time_s -= 1;
                    } else if (keys.press & KEY_DOWN) {
                        editor_config.shift_time_s -= 1;
                    } else if (keys.press & KEY_UP) {
                        editor_config.shift_time_s += 1;
                    }
                    editor_config.shift_time_s = CLAMP(editor_config.shift_time_s, 10, 20 * 60); // 10s - 20m
                    break;
                case PLACEHOLDER:
                    break;
            }
        }
    } else {
        if (keys.press & KEY_UP) {
            editor_index = editor_index_next(editor_index);
        } else if (keys.press & KEY_DOWN) {
            editor_index = editor_index_previous(editor_index);
        } else if (keys.press & KEY_ENTER) {
            editor_edit = true;
        }
    }

    segments[0] = segment_for_int(editor_index);

    if (editor_edit) {
        segments[0] |= 0x80;
    }

    switch (editor_index) {
        case BUZZ_TIME:
            segments[2] = segment_for_int(editor_config.buzz_time_ms / 10000 % 10);
            segments[3] = segment_for_int(editor_config.buzz_time_ms / 1000 % 10) | 0x80;
            segments[4] = segment_for_int(editor_config.buzz_time_ms / 100 % 10) ;
            segments[5] = segment_for_int(editor_config.buzz_time_ms / 10 % 10);
            break;
        case SHIFT_TIME:
            segments[2] = segment_for_int((editor_config.shift_time_s / 60 / 10) % 10);
            segments[3] = segment_for_int((editor_config.shift_time_s / 60 /  1) % 10) | 0x80;
            segments[4] = segment_for_int((editor_config.shift_time_s % 60 / 10) % 10);
            segments[5] = segment_for_int((editor_config.shift_time_s % 60 /  1) % 10);
            break;
        case PLACEHOLDER:
            break;
    }
}
