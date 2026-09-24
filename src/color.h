#pragma once

#include <string>

// American roulette: 38 pockets total.
// 18 Red, 18 Black, 2 Green (0 and 00).
enum class Color { Red, Black, Green };

inline char to_char(Color c) {
    switch (c) {
        case Color::Red:   return 'R';
        case Color::Black: return 'B';
        case Color::Green: return 'G';
    }
    return '?';
}

inline std::string to_string(Color c) {
    switch (c) {
        case Color::Red:   return "Red";
        case Color::Black: return "Black";
        case Color::Green: return "Green";
    }
    return "Unknown";
}
