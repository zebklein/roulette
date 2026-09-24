"""Turns roulette_sim's CSV output into charts about regression to the mean.

Usage:
    pip install -r requirements.txt
    python scripts/visualize.py --trials-csv output/trials.csv \
        --summary-csv output/summary.csv --output-dir plots/
"""

import argparse
import csv
import os
from collections import defaultdict

import matplotlib.pyplot as plt
import numpy as np

# Fixed categorical color order (see dataviz palette: blue, orange, aqua,
# yellow), matched to the C++ strategy factory's own fixed name order so a
# strategy always gets the same color across every chart.
STRATEGY_COLORS = {
    "flat": "#2a78d6",
    "fallacy": "#eb6834",
    "martingale": "#1baf7a",
    "random": "#eda100",
}
STRATEGY_LABELS = {
    "flat": "Flat betting (control)",
    "fallacy": "Gambler's fallacy",
    "martingale": "Martingale",
    "random": "Random color (control)",
}

SURFACE = "#fcfcfb"
INK_PRIMARY = "#0b0b0b"
INK_SECONDARY = "#52514e"
INK_MUTED = "#898781"
GRIDLINE = "#e1e0d9"
BASELINE = "#c3c2b7"

plt.rcParams.update({
    "font.family": "sans-serif",
    "font.sans-serif": ["Segoe UI", "DejaVu Sans", "Arial"],
    "figure.facecolor": SURFACE,
    "axes.facecolor": SURFACE,
    "axes.edgecolor": BASELINE,
    "axes.labelcolor": INK_SECONDARY,
    "text.color": INK_PRIMARY,
    "xtick.color": INK_MUTED,
    "ytick.color": INK_MUTED,
    "grid.color": GRIDLINE,
    "axes.grid": True,
    "grid.linewidth": 0.8,
    "axes.axisbelow": True,
})


def _read_csv(path):
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


_INT_FIELDS = {"round_count", "trial_index", "initial_total", "final_total", "num_trials",
               "busted", "bust_round", "biggest_loss", "num_red", "num_black", "num_green"}
_FLOAT_FIELDS = {
    "percent_profit", "total_wagered", "return_on_wagered_percent",
    "mean_percent_profit", "stddev_percent_profit", "stderr_percent_profit",
    "ci95_lower", "ci95_upper", "mean_return_on_wagered_percent",
    "stddev_return_on_wagered_percent", "stderr_return_on_wagered_percent",
    "ci95_wagered_lower", "ci95_wagered_upper", "pooled_return_on_wagered_percent",
    "total_wagered_across_trials", "bust_rate", "mean_final_total",
    "theoretical_house_edge_percent",
}


def _coerce(rows):
    for row in rows:
        for key in list(row.keys()):
            if key in _INT_FIELDS:
                row[key] = int(row[key])
            elif key in _FLOAT_FIELDS:
                row[key] = float(row[key])
    return rows


def load_summary(path):
    return _coerce(_read_csv(path))


def load_trials(path):
    return _coerce(_read_csv(path))


def _group_by_strategy(rows):
    grouped = defaultdict(list)
    for row in rows:
        grouped[row["strategy"]].append(row)
    for strategy_rows in grouped.values():
        strategy_rows.sort(key=lambda r: r["round_count"])
    return grouped


def _strategy_order(rows):
    seen = list(dict.fromkeys(row["strategy"] for row in rows))
    return [s for s in STRATEGY_COLORS if s in seen] + [s for s in seen if s not in STRATEGY_COLORS]


def plot_convergence(summary_rows, out_dir):
    """Two panels: return-per-dollar-wagered (converges to the house edge
    regardless of strategy) vs. percent profit relative to starting bankroll
    (does not converge -- it trends toward gambler's ruin). Same data,
    two ways of measuring it, very different pictures -- that contrast is
    the point.
    """
    grouped = _group_by_strategy(summary_rows)
    theoretical_edge = summary_rows[0]["theoretical_house_edge_percent"]

    fig, (ax_top, ax_bottom) = plt.subplots(2, 1, figsize=(9, 9), sharex=True)

    for strategy in _strategy_order(summary_rows):
        rows = grouped[strategy]
        x = [r["round_count"] for r in rows]
        y_pooled = [r["pooled_return_on_wagered_percent"] for r in rows]
        y_mean_pct = [r["mean_percent_profit"] for r in rows]
        color = STRATEGY_COLORS.get(strategy, INK_SECONDARY)
        label = STRATEGY_LABELS.get(strategy, strategy)

        ax_top.plot(x, y_pooled, color=color, linewidth=2, marker="o", markersize=8, label=label)
        ax_bottom.plot(x, y_mean_pct, color=color, linewidth=2, marker="o", markersize=8,
                        label=label)

    ax_top.axhline(theoretical_edge, color=INK_MUTED, linewidth=1.5, linestyle="--",
                    label=f"Theoretical house edge ({theoretical_edge:.2f}%)")
    ax_top.set_xscale("log")
    ax_top.set_ylabel("Return per $ wagered (%)")
    ax_top.set_title("Return per dollar wagered converges to the house edge\n"
                      "-- for every strategy, regardless of pattern", fontsize=12,
                      color=INK_PRIMARY)
    ax_top.legend(fontsize=9, frameon=False)

    ax_bottom.set_xscale("log")
    ax_bottom.set_xlabel("Rounds per trial (log scale)")
    ax_bottom.set_ylabel("Mean profit vs. starting bankroll (%)")
    ax_bottom.set_title("...but profit relative to starting cash does NOT converge --\n"
                         "it trends toward gambler's ruin as rounds grow", fontsize=12,
                         color=INK_PRIMARY)

    fig.tight_layout()
    path = os.path.join(out_dir, "convergence.png")
    fig.savefig(path, dpi=150)
    plt.close(fig)
    return path


def plot_profit_distributions(trial_rows, out_dir, round_count=None):
    """Histograms of return-per-dollar-wagered per strategy at a single
    round_count, showing that spread (risk) differs sharply by strategy even
    though they're all centered on the same house edge.
    """
    available_counts = sorted(set(r["round_count"] for r in trial_rows))
    if round_count is None:
        round_count = available_counts[len(available_counts) // 2]

    strategies = _strategy_order(trial_rows)
    fig, axes = plt.subplots(len(strategies), 1, figsize=(8, 2.2 * len(strategies)), sharex=True)
    if len(strategies) == 1:
        axes = [axes]

    for ax, strategy in zip(axes, strategies):
        values = [r["return_on_wagered_percent"] for r in trial_rows
                  if r["strategy"] == strategy and r["round_count"] == round_count]
        color = STRATEGY_COLORS.get(strategy, INK_SECONDARY)
        ax.hist(values, bins=40, color=color, edgecolor=SURFACE, linewidth=0.5)
        ax.axvline(0, color=BASELINE, linewidth=1)
        ax.set_ylabel(STRATEGY_LABELS.get(strategy, strategy), fontsize=9, rotation=0,
                       ha="right", va="center")
        ax.grid(axis="y", visible=False)

    axes[-1].set_xlabel("Return per $ wagered (%), single trial")
    fig.suptitle(f"Distribution of outcomes at {round_count:,} rounds per trial",
                  fontsize=12, color=INK_PRIMARY)
    fig.tight_layout(rect=(0, 0, 1, 0.96))
    path = os.path.join(out_dir, "profit_distributions.png")
    fig.savefig(path, dpi=150)
    plt.close(fig)
    return path


def plot_bust_risk(summary_rows, out_dir):
    """Grouped bar chart of bust rate by strategy x round_count -- the risk
    side of the story that the house-edge convergence chart doesn't show.
    """
    grouped = _group_by_strategy(summary_rows)
    strategies = _strategy_order(summary_rows)
    round_counts = sorted(set(r["round_count"] for r in summary_rows))

    x = np.arange(len(round_counts))
    n = len(strategies)
    width = 0.8 / n

    fig, ax = plt.subplots(figsize=(9, 5))
    for i, strategy in enumerate(strategies):
        by_round = {r["round_count"]: r["bust_rate"] for r in grouped[strategy]}
        heights = [by_round.get(rc, 0.0) * 100.0 for rc in round_counts]
        offset = (i - (n - 1) / 2) * width
        ax.bar(x + offset, heights, width=width, color=STRATEGY_COLORS.get(strategy, INK_SECONDARY),
               label=STRATEGY_LABELS.get(strategy, strategy))

    ax.set_xticks(x)
    ax.set_xticklabels([f"{rc:,}" for rc in round_counts])
    ax.set_xlabel("Rounds per trial")
    ax.set_ylabel("Bust rate (%)")
    ax.set_title("Risk of going broke differs sharply by strategy\n"
                  "-- even though expected return per dollar wagered does not",
                  fontsize=12, color=INK_PRIMARY)
    ax.legend(fontsize=9, frameon=False)
    ax.grid(axis="x", visible=False)

    fig.tight_layout()
    path = os.path.join(out_dir, "bust_risk.png")
    fig.savefig(path, dpi=150)
    plt.close(fig)
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trials-csv", default="output/trials.csv")
    parser.add_argument("--summary-csv", default="output/summary.csv")
    parser.add_argument("--output-dir", default="plots")
    parser.add_argument("--round-count-for-histogram", type=int, default=None,
                         help="Which round_count to use for the distribution chart "
                              "(default: the middle of the available values)")
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    summary_rows = load_summary(args.summary_csv)
    trial_rows = load_trials(args.trials_csv)

    paths = [
        plot_convergence(summary_rows, args.output_dir),
        plot_profit_distributions(trial_rows, args.output_dir, args.round_count_for_histogram),
        plot_bust_risk(summary_rows, args.output_dir),
    ]
    for path in paths:
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
