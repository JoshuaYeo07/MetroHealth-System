#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <iostream>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

/**
 * @namespace Console
 * @brief ANSI colour codes and a figlet-style ASCII logo for the CLI.
 * Colour codes are written as separate stream insertions, so they never affect std::setw alignment.
 */
namespace Console {
    constexpr const char* RESET   = "\033[0m";
    constexpr const char* BOLD    = "\033[1m";
    constexpr const char* DIM     = "\033[2m";
    constexpr const char* RED     = "\033[91m";
    constexpr const char* GREEN   = "\033[92m";
    constexpr const char* YELLOW  = "\033[93m";
    constexpr const char* BLUE    = "\033[94m";
    constexpr const char* MAGENTA = "\033[95m";
    constexpr const char* CYAN    = "\033[96m";

    // Windows consoles need virtual terminal processing switched on before ANSI codes render
    inline void enableColors() {
#ifdef _WIN32
        HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (handle != INVALID_HANDLE_VALUE && GetConsoleMode(handle, &mode)) {
            SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
#endif
    }

    // "MetroHealth" rendered with the pyfiglet "standard" font (plain ASCII, safe on any console)
    inline void printLogo(const std::string& subtitle) {
        std::cout << "\n" << BOLD << CYAN << R"LOGO(
 __  __      _             _   _            _ _   _
|  \/  | ___| |_ _ __ ___ | | | | ___  __ _| | |_| |__
| |\/| |/ _ \ __| '__/ _ \| |_| |/ _ \/ _` | | __| '_ \
| |  | |  __/ |_| | | (_) |  _  |  __/ (_| | | |_| | | |
|_|  |_|\___|\__|_|  \___/|_| |_|\___|\__,_|_|\__|_| |_|
)LOGO" << RESET;
        std::cout << YELLOW << "  " << subtitle << RESET << "\n";
    }
}

#endif // CONSOLE_HPP
