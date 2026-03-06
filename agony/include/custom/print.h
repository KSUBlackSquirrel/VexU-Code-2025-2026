#ifndef CUSTOMPRINT_H_
#define CUSTOMPRINT_H_

#include <cstdarg>
#include <cstdio>
#include "api.h"

namespace customPrint {
// Print to console (serial / RTT)
inline void printf(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    // Sanitize output: replace non-printable or non-ASCII bytes to avoid
    // terminal decode crashes (pros-cli may choke on invalid UTF-8 bytes).
    char out[512];
    size_t in_len = strlen(buf);
    size_t out_pos = 0;
    for (size_t i = 0; i < in_len && out_pos + 1 < sizeof(out); ++i) {
        unsigned char c = static_cast<unsigned char>(buf[i]);
        // allow common printable ASCII and newline/tab
        if (c >= 0x20 && c <= 0x7E) {
            out[out_pos++] = static_cast<char>(c);
        } else if (c == '\n' || c == '\r' || c == '\t') {
            out[out_pos++] = static_cast<char>(c);
        } else {
            // replace any control or non-ASCII byte with '?'
            out[out_pos++] = '?';
        }
    }
    // ensure newline-termination
    if (out_pos == 0 || out[out_pos - 1] != '\n') {
        if (out_pos + 1 < sizeof(out)) out[out_pos++] = '\n';
    }
    out[out_pos] = '\0';
    ::printf("%s", out);
}

inline void clearScreen(int line) {
    int y = line * 20;  // Approximate line height
    pros::screen::set_eraser(pros::Color::black);
    pros::screen::fill_rect(0, y, 480, y + 20);
}

// Print to the V5 screen at a specific line
inline void screenPrint(int line, const char* fmt, ...) {
    clearScreen(line);
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    pros::screen::print(pros::E_TEXT_MEDIUM, line, "%s", buf);
}
} // namespace customPrint

#endif // CUSTOMPRINT_H_
