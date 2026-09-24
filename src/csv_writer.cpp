#include "csv_writer.h"

#include <fstream>
#include <stdexcept>

void write_trials_csv(const std::string& path, const std::vector<TrialResult>& trials) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("Could not open " + path + " for writing");

    out << "strategy,round_count,trial_index,initial_total,final_total,percent_profit,"
           "total_wagered,return_on_wagered_percent,"
           "busted,bust_round,biggest_loss,num_red,num_black,num_green\n";
    for (const auto& t : trials) {
        out << t.strategy << ',' << t.round_count << ',' << t.trial_index << ','
            << t.initial_total << ',' << t.final_total << ',' << t.percent_profit << ','
            << t.total_wagered << ',' << t.return_on_wagered_percent << ','
            << (t.busted ? 1 : 0) << ',' << t.bust_round << ',' << t.biggest_loss << ','
            << t.num_red << ',' << t.num_black << ',' << t.num_green << '\n';
    }
}

void write_summary_csv(const std::string& path, const std::vector<SummaryStats>& summaries) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("Could not open " + path + " for writing");

    out << "strategy,round_count,num_trials,mean_percent_profit,stddev_percent_profit,"
           "stderr_percent_profit,ci95_lower,ci95_upper,"
           "mean_return_on_wagered_percent,stddev_return_on_wagered_percent,"
           "stderr_return_on_wagered_percent,ci95_wagered_lower,ci95_wagered_upper,"
           "pooled_return_on_wagered_percent,total_wagered_across_trials,"
           "bust_rate,mean_final_total,theoretical_house_edge_percent\n";
    for (const auto& s : summaries) {
        out << s.strategy << ',' << s.round_count << ',' << s.num_trials << ','
            << s.mean_percent_profit << ',' << s.stddev_percent_profit << ','
            << s.stderr_percent_profit << ',' << s.ci95_lower << ',' << s.ci95_upper << ','
            << s.mean_return_on_wagered_percent << ',' << s.stddev_return_on_wagered_percent
            << ',' << s.stderr_return_on_wagered_percent << ',' << s.ci95_wagered_lower << ','
            << s.ci95_wagered_upper << ',' << s.pooled_return_on_wagered_percent << ','
            << s.total_wagered_across_trials << ',' << s.bust_rate << ','
            << s.mean_final_total << ',' << s.theoretical_house_edge_percent << '\n';
    }
}
