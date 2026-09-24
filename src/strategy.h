#pragma once

#include <string>

#include "color.h"

// A betting strategy picks a color and a bet size each round, and reacts to
// results. All strategies place even-money bets (Red or Black); none can
// change the expected value of an independent random game, only the shape
// of variance and bust risk. See README.md for why.
class BettingStrategy {
public:
    virtual ~BettingStrategy() = default;

    // Reset all per-trial state before a fresh trial begins.
    virtual void reset(int initial_wager) = 0;

    // The amount to wager this round, given the player's current cash.
    virtual int next_bet(int current_total) const = 0;

    // The color to bet on this round.
    virtual Color color_choice() const = 0;

    // Informs the strategy of the round's outcome so it can update state.
    virtual void record_result(Color result, bool won) = 0;

    virtual std::string name() const = 0;
};
