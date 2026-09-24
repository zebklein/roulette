#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "csv_writer.h"
#include "simulation.h"
#include "statistics.h"
#include "strategies.h"
#include "wheel.h"

namespace fs = std::filesystem;

struct Config {
    std::vector<std::string> strategies = all_strategy_names();
    std::vector<int> round_counts = {10, 100, 1000, 3000, 10000};
    int trials = 1000;
    int initial_total = 1000;
    int initial_wager = 10;
    double growth_rate = 1.5;
    std::optional<unsigned int> seed;
    std::string output_dir = "output";
    std::optional<std::string> trace_path;
};

namespace {

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) parts.push_back(item);
    }
    return parts;
}

void print_usage() {
    std::cout <<
        "roulette_sim - simulate roulette betting strategies to study regression to the mean\n\n"
        "Usage: roulette_sim [options]\n\n"
        "Options:\n"
        "  --strategies <list>     Comma-separated strategy names (default: all)\n"
        "                          Available: flat, fallacy, martingale, random\n"
        "  --round-counts <list>   Comma-separated round counts per trial\n"
        "                          (default: 10,100,1000,3000,10000)\n"
        "  --trials <N>            Trials per (strategy, round-count) combo (default: 1000)\n"
        "  --initial-total <int>   Starting cash (default: 1000)\n"
        "  --initial-wager <int>   Base bet (default: 10)\n"
        "  --growth-rate <double>  Gambler's-fallacy escalation multiplier (default: 1.5)\n"
        "  --seed <uint>           RNG seed, for reproducible runs (default: random)\n"
        "  --output-dir <path>     Where trials.csv/summary.csv are written (default: output)\n"
        "  --trace <path>          Write a round-by-round log of the first trial of each\n"
        "                          combo to this file (default: off)\n"
        "  --help                  Show this message\n\n"
        "Example:\n"
        "  roulette_sim --trials 2000 --round-counts 100,10000 --seed 42\n";
}

// Parses `argv` into a Config. Returns std::nullopt if --help was given
// (usage already printed) or on error (message already printed).
std::optional<Config> parse_args(int argc, char* argv[]) {
    Config config;
    std::vector<std::string> args(argv + 1, argv + argc);

    auto next_value = [&](size_t& i) -> std::optional<std::string> {
        if (i + 1 >= args.size()) return std::nullopt;
        return args[++i];
    };

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "--help" || arg == "-h") {
            print_usage();
            return std::nullopt;
        } else if (arg == "--strategies") {
            auto v = next_value(i);
            if (!v) { std::cerr << "Missing value for --strategies\n"; return std::nullopt; }
            config.strategies = split(*v, ',');
        } else if (arg == "--round-counts") {
            auto v = next_value(i);
            if (!v) { std::cerr << "Missing value for --round-counts\n"; return std::nullopt; }
            config.round_counts.clear();
            for (const auto& part : split(*v, ',')) config.round_counts.push_back(std::stoi(part));
        } else if (arg == "--trials") {
            auto v = next_value(i);
            if (!v) { std::cerr << "Missing value for --trials\n"; return std::nullopt; }
            config.trials = std::stoi(*v);
        } else if (arg == "--initial-total") {
            auto v = next_value(i);
            if (!v) { std::cerr << "Missing value for --initial-total\n"; return std::nullopt; }
            config.initial_total = std::stoi(*v);
        } else if (arg == "--initial-wager") {
            auto v = next_value(i);
            if (!v) { std::cerr << "Missing value for --initial-wager\n"; return std::nullopt; }
            config.initial_wager = std::stoi(*v);
        } else if (arg == "--growth-rate") {
            auto v = next_value(i);
            if (!v) { std::cerr << "Missing value for --growth-rate\n"; return std::nullopt; }
            config.growth_rate = std::stod(*v);
        } else if (arg == "--seed") {
            auto v = next_value(i);
            if (!v) { std::cerr << "Missing value for --seed\n"; return std::nullopt; }
            config.seed = static_cast<unsigned int>(std::stoul(*v));
        } else if (arg == "--output-dir") {
            auto v = next_value(i);
            if (!v) { std::cerr << "Missing value for --output-dir\n"; return std::nullopt; }
            config.output_dir = *v;
        } else if (arg == "--trace") {
            auto v = next_value(i);
            if (!v) { std::cerr << "Missing value for --trace\n"; return std::nullopt; }
            config.trace_path = *v;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n\n";
            print_usage();
            return std::nullopt;
        }
    }

    for (const auto& name : config.strategies) {
        if (std::find(all_strategy_names().begin(), all_strategy_names().end(), name) ==
            all_strategy_names().end()) {
            std::cerr << "Unknown strategy: " << name << "\n\n";
            print_usage();
            return std::nullopt;
        }
    }

    return config;
}

void write_trace(std::ofstream& trace, const std::string& strategy, int round_count,
                  BettingStrategy& strategy_obj, Wheel& wheel, int initial_total,
                  int initial_wager) {
    trace << "-- " << strategy << ", " << round_count << " rounds --\n";
    strategy_obj.reset(initial_wager);
    int current_total = initial_total;
    for (int round = 0; round < round_count; ++round) {
        const int bet = strategy_obj.next_bet(current_total);
        if (bet > current_total) {
            trace << "  Round " << round + 1 << ": BUST (needed " << bet << ", had "
                  << current_total << ")\n";
            break;
        }
        const Color choice = strategy_obj.color_choice();
        const Color outcome = wheel.spin();
        const bool won = (outcome == choice);
        current_total += won ? bet : -bet;
        trace << "  Round " << round + 1 << ": bet " << bet << " on " << to_string(choice)
              << ", spin=" << to_string(outcome) << ", " << (won ? "WIN" : "LOSS")
              << ", total=" << current_total << "\n";
        strategy_obj.record_result(outcome, won);
    }
    trace << "\n";
}

void print_console_table(const std::vector<SummaryStats>& summaries) {
    // "mean %" is profit relative to starting bankroll -- intuitive, but it
    // does NOT converge to the house edge as rounds grow (see README).
    // "pooled/$wagered" is total profit / total amount wagered across every
    // trial in the group -- this is the metric that reliably converges to
    // the theoretical house edge via the law of large numbers.
    std::cout << std::left << std::setw(12) << "strategy" << std::right << std::setw(10)
              << "rounds" << std::setw(11) << "mean %" << std::setw(17) << "pooled/$wagered"
              << std::setw(14) << "bust rate" << "\n";
    std::cout << std::string(64, '-') << "\n";
    for (const auto& s : summaries) {
        std::cout << std::left << std::setw(12) << s.strategy << std::right << std::setw(10)
                  << s.round_count << std::setw(11) << std::fixed << std::setprecision(2)
                  << s.mean_percent_profit << std::setw(16)
                  << s.pooled_return_on_wagered_percent << std::setw(13) << std::setprecision(1)
                  << s.bust_rate * 100.0 << "%\n";
    }
    std::cout << "\nTheoretical house edge (return per dollar wagered): " << std::fixed
              << std::setprecision(3) << kTheoreticalHouseEdgePercent << "%\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    auto maybe_config = parse_args(argc, argv);
    if (!maybe_config) return 0;
    Config config = *maybe_config;

    std::mt19937 engine(config.seed.value_or(std::random_device{}()));
    Wheel wheel(engine);

    std::ofstream trace;
    if (config.trace_path) {
        trace.open(*config.trace_path);
        if (!trace) {
            std::cerr << "Could not open trace file: " << *config.trace_path << "\n";
            return 1;
        }
    }

    std::vector<TrialResult> all_trials;
    std::vector<SummaryStats> all_summaries;

    for (const auto& strategy_name : config.strategies) {
        for (int round_count : config.round_counts) {
            auto strategy = create_strategy(strategy_name, config.growth_rate, engine);

            if (trace.is_open()) {
                write_trace(trace, strategy_name, round_count, *strategy, wheel,
                            config.initial_total, config.initial_wager);
            }

            std::vector<TrialResult> trials;
            trials.reserve(config.trials);
            for (int trial_index = 0; trial_index < config.trials; ++trial_index) {
                trials.push_back(run_trial(*strategy, wheel, strategy_name, round_count,
                                            trial_index, config.initial_total,
                                            config.initial_wager));
            }

            all_summaries.push_back(compute_summary(strategy_name, round_count, trials));
            all_trials.insert(all_trials.end(), trials.begin(), trials.end());
        }
    }

    fs::create_directories(config.output_dir);
    const std::string trials_path = (fs::path(config.output_dir) / "trials.csv").string();
    const std::string summary_path = (fs::path(config.output_dir) / "summary.csv").string();
    write_trials_csv(trials_path, all_trials);
    write_summary_csv(summary_path, all_summaries);

    print_console_table(all_summaries);
    std::cout << "\nWrote " << all_trials.size() << " trial rows to " << trials_path << "\n";
    std::cout << "Wrote " << all_summaries.size() << " summary rows to " << summary_path << "\n";

    return 0;
}
