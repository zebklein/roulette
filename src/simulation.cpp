#include "simulation.h"

TrialResult run_trial(BettingStrategy& strategy, Wheel& wheel, const std::string& strategy_name,
                       int round_count, int trial_index, int initial_total, int initial_wager) {
    strategy.reset(initial_wager);

    TrialResult result;
    result.strategy = strategy_name;
    result.round_count = round_count;
    result.trial_index = trial_index;
    result.initial_total = initial_total;

    int current_total = initial_total;
    int current_loss_streak = 0;

    for (int round = 0; round < round_count; ++round) {
        const int bet = strategy.next_bet(current_total);
        if (bet > current_total) {
            result.busted = true;
            result.bust_round = round;
            break;
        }

        result.total_wagered += bet;

        const Color choice = strategy.color_choice();
        const Color outcome = wheel.spin();
        const bool won = (outcome == choice);

        switch (outcome) {
            case Color::Red: ++result.num_red; break;
            case Color::Black: ++result.num_black; break;
            case Color::Green: ++result.num_green; break;
        }

        if (won) {
            current_total += bet;
            current_loss_streak = 0;
        } else {
            current_total -= bet;
            current_loss_streak += bet;
            if (current_loss_streak > result.biggest_loss) {
                result.biggest_loss = current_loss_streak;
            }
        }

        strategy.record_result(outcome, won);
    }

    result.final_total = current_total;
    result.percent_profit =
        ((static_cast<double>(current_total) - initial_total) / initial_total) * 100.0;
    if (result.total_wagered > 0.0) {
        result.return_on_wagered_percent =
            ((static_cast<double>(current_total) - initial_total) / result.total_wagered) * 100.0;
    }
    return result;
}
