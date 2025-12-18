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
    ::printf("%s", buf);
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
