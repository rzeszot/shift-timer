#include "version.h"
#include "segments.h"

void build_version_segments(const char *version, uint8_t out[16]) {
    for (uint8_t i = 0; i < 16; ++i) {
        out[i] = 0x00;
    }

    int pos = 0;
    int last_digit_idx = -1;

    for (const char *p = version; *p && pos < 8; ++p) {
        if (*p == '.') {
            if (last_digit_idx >= 0) {
                out[last_digit_idx] |= segment_dot;
            }
            continue;
        }
        uint8_t seg = segment_for_character(*p);

        int tm_idx = pos * 2;
        out[tm_idx] = seg;
        last_digit_idx = tm_idx;
        pos++;
    }
}


