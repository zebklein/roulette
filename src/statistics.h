#pragma once

#include <string>
#include <vector>

#include "simulation.h"

// American roulette house edge on an even-money bet: the player wins on 18
// of 38 pockets and loses on 20, so expected value per unit bet is
// (18 - 20) / 38 = -1/19 ~= -5.263%. This is the value every strategy's
// mean percent profit should converge toward as round_count grows.
constexpr double kTheoreticalHouseEdgePercent = -100.0 / 19.0;

struct SummaryStats {
    std::string strategy;
    int round_count = 0;
    int num_trials = 0;
    double mean_percent_profit = 0.0;
    double stddev_percent_profit = 0.0;
    double stderr_percent_profit = 0.0;
    double ci95_lower = 0.0;
    double ci95_upper = 0.0;
    // Return per dollar wagered -- the metric that actually converges to
    // the theoretical house edge as round_count grows (see simulation.h).
    // This is the MEAN OF PER-TRIAL RATIOS -- kept for illustration, but it
    // can be unstable when total_wagered varies a lot between trials
    // (classic "mean of ratios is not the ratio of means" pitfall). See
    // pooled_return_on_wagered_percent for the statistically sound version.
    double mean_return_on_wagered_percent = 0.0;
    double stddev_return_on_wagered_percent = 0.0;
    double stderr_return_on_wagered_percent = 0.0;
    double ci95_wagered_lower = 0.0;
    double ci95_wagered_upper = 0.0;
    // sum(final - initial) / sum(total_wagered) across every trial in the
    // group -- a POOLED ratio, not an average of ratios. This is the
    // statistically sound long-run estimate of return per dollar wagered,
    // and converges cleanly to the theoretical house edge because it's
    // effectively an average over every individual bet placed, rather than
    // an average of one (noisy, denominator-sensitive) ratio per trial.
    double pooled_return_on_wagered_percent = 0.0;
    double total_wagered_across_trials = 0.0;
    double bust_rate = 0.0;
    double mean_final_total = 0.0;
    double theoretical_house_edge_percent = kTheoreticalHouseEdgePercent;
};

SummaryStats compute_summary(const std::string& strategy, int round_count,
                              const std::vector<TrialResult>& trials);
