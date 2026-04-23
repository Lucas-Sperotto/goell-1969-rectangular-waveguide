#!/usr/bin/env python3
import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt


DEFAULT_CSVS = [
    "out/fig16_odd_phi0.csv",
    "out/fig16_odd_phi90.csv",
    "out/fig16_even_phi0.csv",
    "out/fig16_even_phi90.csv",
]

STYLE_CYCLE = ["o-", "s--", "^-", "x--", "d-", ".-"]
MARKER_CYCLE = ["o", "s", "^", "x", "d", "."]


def load_rows(path, pmin=0.0, merit_max=None):
    # Cada CSV representa uma das quatro classes do paper:
    # odd/phi0, odd/phi90, even/phi0 e even/phi90.
    rows = []
    with open(path, newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        required = {"branch_id", "B", "Pprime"}
        if reader.fieldnames is None or not required.issubset(reader.fieldnames):
            raise SystemExit(f"{path} precisa ter colunas: branch_id,B,Pprime")

        for row in reader:
            pprime_raw = row["Pprime"].strip()
            if not pprime_raw or pprime_raw.lower() == "nan":
                continue

            pprime = float(pprime_raw)
            if not (pmin <= pprime <= 1.0):
                continue

            merit = None
            merit_raw = row.get("merit", "").strip()
            if merit_raw and merit_raw.lower() != "nan":
                merit = float(merit_raw)
            if merit_max is not None and merit is not None and merit > merit_max:
                continue

            rows.append(
                {
                    "branch_id": int(row["branch_id"]),
                    "B": float(row["B"]),
                    "Pprime": pprime,
                    "merit": merit,
                    "parity": row.get("parity", "").strip(),
                    "phase": row.get("phase", "").strip(),
                }
            )

    return sorted(rows, key=lambda item: (item["branch_id"], item["B"]))


def infer_label(path, rows):
    if rows:
        parity = rows[0].get("parity") or ""
        phase = rows[0].get("phase") or ""
        if parity and phase:
            return f"{parity}/{phase}"
        if parity:
            return parity
    return Path(path).stem


def plot_dataset(ax, path, index, scatter=False, pmin=0.0, merit_max=None):
    rows = load_rows(path, pmin=pmin, merit_max=merit_max)
    if not rows:
        return

    label = infer_label(path, rows)
    if scatter:
        # No modo scatter mostramos todos os minimos locais exportados pelo solver,
        # sem tentar costurar ramos modais entre valores consecutivos de B.
        marker = MARKER_CYCLE[index % len(MARKER_CYCLE)]
        ax.plot(
            [row["B"] for row in rows],
            [row["Pprime"] for row in rows],
            linestyle="None",
            marker=marker,
            markersize=3,
            alpha=0.85,
            label=label,
        )
        return

    style = STYLE_CYCLE[index % len(STYLE_CYCLE)]
    branch_ids = sorted({row["branch_id"] for row in rows})
    for branch_id in branch_ids:
        group = [row for row in rows if row["branch_id"] == branch_id]
        ax.plot(
            [row["B"] for row in group],
            [row["Pprime"] for row in group],
            style,
            lw=1.2,
            ms=3,
            alpha=0.85,
            label=label if branch_id == branch_ids[0] else None,
        )


def parse_args():
    parser = argparse.ArgumentParser(
        description="Plota as curvas B x P' geradas pelo goell_q_solver."
    )
    parser.add_argument("csvs", nargs="*", default=DEFAULT_CSVS)
    parser.add_argument(
        "--title",
        default="Goell CHM - curvas de propagacao",
    )
    parser.add_argument("--save", help="Salva a figura em vez de abrir na tela.")
    parser.add_argument("--scatter", action="store_true", help="Plota todos os pontos sem ligar ramos.")
    parser.add_argument("--pmin", type=float, default=0.0, help="Descarta pontos com P' abaixo deste valor.")
    parser.add_argument("--merit-max", type=float, help="Descarta pontos com merit acima deste valor.")
    parser.add_argument("--xmax", type=float, default=4.0)
    parser.add_argument("--ymax", type=float, default=1.0)
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    fig, ax = plt.subplots(figsize=(8, 5), dpi=140)
    for index, path in enumerate(args.csvs):
        plot_dataset(
            ax,
            path,
            index,
            scatter=args.scatter,
            pmin=args.pmin,
            merit_max=args.merit_max,
        )

    ax.set_xlim(0, args.xmax)
    ax.set_ylim(0, args.ymax)
    ax.set_xlabel("B")
    ax.set_ylabel("Pprime")
    ax.grid(True, alpha=0.3)

    handles, labels = ax.get_legend_handles_labels()
    if handles:
        ax.legend(frameon=False)

    ax.set_title(args.title)
    fig.tight_layout()

    if args.save:
        fig.savefig(args.save, bbox_inches="tight")
    else:
        plt.show()
