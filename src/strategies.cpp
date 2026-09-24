#include "strategies.h"

#include <stdexcept>

// ---- FlatBettingStrategy ----------------------------------------------

void FlatBettingStrategy::reset(int initial_wager) { base_wager_ = initial_wager; }

int FlatBettingStrategy::next_bet(int /*current_total*/) const { return base_wager_; }

Color FlatBettingStrategy::color_choice() const { return Color::Red; }

void FlatBettingStrategy::record_result(Color /*result*/, bool /*won*/) {}

// ---- GamblersFallacyStrategy -------------------------------------------

void GamblersFallacyStrategy::reset(int initial_wager) {
    base_wager_ = initial_wager;
    current_bet_ = initial_wager;
    loss_streak_total_ = 0.0;
    num_red_ = 0;
    num_black_ = 0;
}

int GamblersFallacyStrategy::next_bet(int /*current_total*/) const {
    return static_cast<int>(current_bet_);
}

Color GamblersFallacyStrategy::color_choice() const {
    // Bet against whichever color has led so far -- the gambler's fallacy.
    return (num_red_ > num_black_) ? Color::Black : Color::Red;
}

void GamblersFallacyStrategy::record_result(Color result, bool won) {
    if (result == Color::Red) ++num_red_;
    else if (result == Color::Black) ++num_black_;

    if (won) {
        current_bet_ = base_wager_;
        loss_streak_total_ = 0.0;
    } else {
        loss_streak_total_ += current_bet_;
        current_bet_ = loss_streak_total_ * growth_rate_;
    }
}

// ---- MartingaleStrategy --------------------------------------------------

void MartingaleStrategy::reset(int initial_wager) {
    base_wager_ = initial_wager;
    current_bet_ = initial_wager;
}

int MartingaleStrategy::next_bet(int /*current_total*/) const { return current_bet_; }

Color MartingaleStrategy::color_choice() const { return Color::Red; }

void MartingaleStrategy::record_result(Color /*result*/, bool won) {
    current_bet_ = won ? base_wager_ : current_bet_ * 2;
}

// ---- RandomStrategy --------------------------------------------------

void RandomStrategy::reset(int initial_wager) {
    base_wager_ = initial_wager;
    current_color_ = color_dist_(engine_) == 0 ? Color::Red : Color::Black;
}

int RandomStrategy::next_bet(int /*current_total*/) const { return base_wager_; }

Color RandomStrategy::color_choice() const { return current_color_; }

void RandomStrategy::record_result(Color /*result*/, bool /*won*/) {
    current_color_ = color_dist_(engine_) == 0 ? Color::Red : Color::Black;
}

// ---- factory --------------------------------------------------------

std::vector<std::string> all_strategy_names() {
    return {"flat", "fallacy", "martingale", "random"};
}

std::unique_ptr<BettingStrategy> create_strategy(const std::string& name, double growth_rate,
                                                  std::mt19937& engine) {
    if (name == "flat") return std::make_unique<FlatBettingStrategy>();
    if (name == "fallacy") return std::make_unique<GamblersFallacyStrategy>(growth_rate);
    if (name == "martingale") return std::make_unique<MartingaleStrategy>();
    if (name == "random") return std::make_unique<RandomStrategy>(engine);
    throw std::invalid_argument("Unknown strategy: " + name);
}
