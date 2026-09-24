#pragma once

#include <string>
#include <vector>

#include "simulation.h"
#include "statistics.h"

void write_trials_csv(const std::string& path, const std::vector<TrialResult>& trials);
void write_summary_csv(const std::string& path, const std::vector<SummaryStats>& summaries);
