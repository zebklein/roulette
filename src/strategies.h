#pragma once

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "strategy.h"

// Control group: fixed bet, fixed color, no reaction to history at all.
class FlatBettingStrategy : public BettingStrategy {
public:
    void reset(int initial_wager) override;
    int next_bet(int current_total) const override;
    Color color_choice() const override;
    void record_result(Color result, bool won) override;
    std::string name() const override { return "flat"; }

private:
    int base_wager_ = 0;
};

// The original 2021 algorithm: bet against whichever color has led so far,
// and escalate the bet after a loss proportional to the cumulative loss in
// the current streak. This is the gambler's fallacy in code form -- it
// assumes red/black must "balance out," which is false for independent
// spins.
class GamblersFallacyStrategy : public BettingStrategy {
public:
    explicit GamblersFallacyStrategy(double growth_rate) : growth_rate_(growth_rate) {}

    void reset(int initial_wager) override;
    int next_bet(int current_total) const override;
    Color color_choice() const override;
    void record_result(Color result, bool won) override;
    std::string name() const override { return "fallacy"; }

private:
    double growth_rate_;
    int base_wager_ = 0;
    double current_bet_ = 0.0;
    double loss_streak_total_ = 0.0;
    int num_red_ = 0;
    int num_black_ = 0;
};

// The textbook Martingale system: always bet the same color, double the bet
// after every loss, reset to the base bet after a win.
class MartingaleStrategy : public BettingStrategy {
public:
    void reset(int initial_wager) override;
    int next_bet(int current_total) const override;
    Color color_choice() const override;
    void record_result(Color result, bool won) override;
    std::string name() const override { return "martingale"; }

private:
    int base_wager_ = 0;
    int current_bet_ = 0;
};

// Control group: fixed bet, but the color bet each round is uniformly
// random rather than following any pattern.
class RandomStrategy : public BettingStrategy {
public:
    explicit RandomStrategy(std::mt19937& engine) : engine_(engine), color_dist_(0, 1) {}

    void reset(int initial_wager) override;
    int next_bet(int current_total) const override;
    Color color_choice() const override;
    void record_result(Color result, bool won) override;
    std::string name() const override { return "random"; }

private:
    std::mt19937& engine_;
    std::uniform_int_distribution<int> color_dist_;
    int base_wager_ = 0;
    Color current_color_ = Color::Red;
};

std::vector<std::string> all_strategy_names();

std::unique_ptr<BettingStrategy> create_strategy(const std::string& name, double growth_rate,
                                                  std::mt19937& engine);
