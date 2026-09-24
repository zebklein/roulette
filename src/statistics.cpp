#include "statistics.h"

#include <cmath>

namespace {

struct MeanCi {
    double mean = 0.0;
    double stddev = 0.0;
    double stderr_ = 0.0;
    double ci_lower = 0.0;
    double ci_upper = 0.0;
};

template <typename ValueOf>
MeanCi mean_with_ci(const std::vector<TrialResult>& trials, ValueOf value_of) {
    MeanCi result;
    const double n = static_cast<double>(trials.size());

    double sum = 0.0;
    for (const auto& t : trials) sum += value_of(t);
    result.mean = sum / n;

    double sum_sq_dev = 0.0;
    for (const auto& t : trials) {
        const double dev = value_of(t) - result.mean;
        sum_sq_dev += dev * dev;
    }
    // Sample standard deviation (n-1); undefined for a single trial.
    result.stddev = (n > 1) ? std::sqrt(sum_sq_dev / (n - 1)) : 0.0;
    result.stderr_ = result.stddev / std::sqrt(n);

    // Normal approximation to a 95% CI; reasonable once num_trials is at
    // least in the hundreds, which is the intended default usage here.
    constexpr double kZ95 = 1.96;
    result.ci_lower = result.mean - kZ95 * result.stderr_;
    result.ci_upper = result.mean + kZ95 * result.stderr_;

    return result;
}

}  // namespace

SummaryStats compute_summary(const std::string& strategy, int round_count,
                              const std::vector<TrialResult>& trials) {
    SummaryStats stats;
    stats.strategy = strategy;
    stats.round_count = round_count;
    stats.num_trials = static_cast<int>(trials.size());
    if (trials.empty()) return stats;

    const double n = static_cast<double>(trials.size());

    double sum_final = 0.0;
    double sum_profit = 0.0;
    double sum_wagered = 0.0;
    int num_busted = 0;
    for (const auto& t : trials) {
        sum_final += t.final_total;
        sum_profit += (t.final_total - t.initial_total);
        sum_wagered += t.total_wagered;
        if (t.busted) ++num_busted;
    }
    stats.mean_final_total = sum_final / n;
    stats.bust_rate = num_busted / n;
    stats.total_wagered_across_trials = sum_wagered;
    stats.pooled_return_on_wagered_percent =
        (sum_wagered > 0.0) ? (sum_profit / sum_wagered) * 100.0 : 0.0;

    const auto profit = mean_with_ci(trials, [](const TrialResult& t) { return t.percent_profit; });
    stats.mean_percent_profit = profit.mean;
    stats.stddev_percent_profit = profit.stddev;
    stats.stderr_percent_profit = profit.stderr_;
    stats.ci95_lower = profit.ci_lower;
    stats.ci95_upper = profit.ci_upper;

    const auto wagered = mean_with_ci(
        trials, [](const TrialResult& t) { return t.return_on_wagered_percent; });
    stats.mean_return_on_wagered_percent = wagered.mean;
    stats.stddev_return_on_wagered_percent = wagered.stddev;
    stats.stderr_return_on_wagered_percent = wagered.stderr_;
    stats.ci95_wagered_lower = wagered.ci_lower;
    stats.ci95_wagered_upper = wagered.ci_upper;

    return stats;
}
