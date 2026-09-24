#pragma once

#include <random>

#include "color.h"

// A single American roulette wheel: 1-18 and 20-36 alternate red/black in the
// real wheel, but for simulation purposes only the color distribution matters
// (18 Red, 18 Black, 2 Green), so pockets are modeled as a flat 1-38 draw.
class Wheel {
public:
    explicit Wheel(std::mt19937& engine);

    Color spin();

private:
    std::mt19937& engine_;
    std::uniform_int_distribution<int> dist_;
};
