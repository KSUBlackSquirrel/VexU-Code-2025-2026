/**
 * @file print.h
 * @brief Utility functions for printing to terminal and V5 brain screen
 * 
 * Provides convenient printf-style functions for debugging and telemetry.
 * Use printf() for terminal output (visible in PROS terminal) and screenPrint()
 * for V5 brain screen output (visible on robot).
 */
#ifndef CUSTOMPRINT_H_
#define CUSTOMPRINT_H_

#include <cstdarg>
#include <cstdio>
#include "api.h"

namespace customPrint {
/**
 * @brief Print formatted text to terminal (serial / RTT)
 * @param fmt Printf-style format string
 * @param ... Variable arguments for format string
 * 
 * Output appears in PROS terminal when connected to computer.
 * Useful for debugging and logging during development.
 * Example: customPrint::printf("Temperature: %.1f\n", temp)
 */
inline void printf(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ::printf("%s", buf);
}

/**
 * @brief Clear a line on the V5 brain screen
 * @param line Line number to clear (0-based)
 * 
 * Used internally by screenPrint() to clear old text before printing new text.
 */
inline void clearScreen(int line) {
    int y = line * 20;  // Approximate line height
    pros::screen::set_eraser(pros::Color::black);
    pros::screen::fill_rect(0, y, 480, y + 20);
}

/**
 * @brief Print formatted text to V5 brain screen at a specific line
 * @param line Line number to print on (0-based, ~12 lines available)
 * @param fmt Printf-style format string
 * @param ... Variable arguments for format string
 * 
 * Output appears on the V5 brain touchscreen. Useful for telemetry visible
 * during matches when computer is not connected.
 * Example: customPrint::screenPrint(0, "Battery: %.1fV", voltage)
 */
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
