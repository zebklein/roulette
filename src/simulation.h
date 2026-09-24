#pragma once

#include "strategy.h"
#include "wheel.h"

struct TrialResult {
    std::string strategy;
    int round_count = 0;
    int trial_index = 0;
    int initial_total = 0;
    int final_total = 0;
    double percent_profit = 0.0;  // (final - initial) / initial, relative to starting bankroll
    double total_wagered = 0.0;   // sum of every bet placed in the trial
    // (final - initial) / total_wagered. Unlike percent_profit, this stays
    // well-behaved as round_count grows and is the metric that converges
    // to the house edge -- see README for why the two behave so differently.
    double return_on_wagered_percent = 0.0;
    bool busted = false;
    int bust_round = -1;
    int biggest_loss = 0;
    int num_red = 0;
    int num_black = 0;
    int num_green = 0;
};

// Runs one trial: up to `round_count` rounds, betting per `strategy` each
// round, stopping early if the player can't cover the next required bet
// (a "bust"). Bust-checking is centralized here so every strategy is held
// to the exact same rule.
TrialResult run_trial(BettingStrategy& strategy, Wheel& wheel, const std::string& strategy_name,
                       int round_count, int trial_index, int initial_total, int initial_wager);
