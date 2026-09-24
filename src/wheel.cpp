#include "wheel.h"

Wheel::Wheel(std::mt19937& engine) : engine_(engine), dist_(1, 38) {}

Color Wheel::spin() {
    const int pocket = dist_(engine_);
    if (pocket <= 18) return Color::Red;
    if (pocket <= 36) return Color::Black;
    return Color::Green;  // 37 = 0, 38 = 00
}
