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
X_TICKS = [0.4 * step for step in range(11)]

ARTICLE_ANNOTATIONS = {
    "fig16": [
        {"text": r"$E^y_{11}, E^x_{11}$", "xy": (1.75, 0.66), "xytext": (1.15, 0.71)},
        {
            "text": r"$E^y_{12}, E^x_{12}$" "\n" r"$E^y_{21}, E^x_{21}$",
            "xy": (1.72, 0.57),
            "xytext": (2.08, 0.51),
        },
        {"text": r"$E^y_{31}, E^x_{13}$", "xy": (2.38, 0.06), "xytext": (2.18, 0.24)},
        {"text": r"$E^x_{31}, E^y_{13}$", "xy": (2.46, 0.03), "xytext": (1.96, 0.09)},
        {"text": r"$E^y_{22}, E^x_{22}$", "xy": (2.86, 0.33), "xytext": (3.04, 0.44)},
        {
            "text": r"$E^y_{32}, E^x_{23}$" "\n" r"$E^x_{32}, E^y_{23}$",
            "xy": (3.20, 0.17),
            "xytext": (3.52, 0.12),
        },
    ],
    "fig17": [
        {"text": r"$E^y_{11}, E^x_{11}$", "xy": (1.56, 0.67), "xytext": (0.76, 0.68)},
        {"text": r"$E^y_{21}, E^x_{21}$", "xy": (2.25, 0.74), "xytext": (1.50, 0.86)},
        {"text": r"$E^y_{31}, E^x_{31}$", "xy": (3.74, 0.75), "xytext": (2.78, 0.92)},
        {"text": r"$E^y_{12}, E^x_{12}$", "xy": (3.08, 0.67), "xytext": (3.50, 0.53)},
        {"text": r"$E^y_{22}, E^x_{22}$", "xy": (2.34, 0.31), "xytext": (2.88, 0.34)},
        {"text": r"$E^y_{41}, E^x_{41}$", "xy": (2.10, 0.34), "xytext": (2.46, 0.16)},
    ],
    "fig18": [
        {"text": r"$E^y_{11}, E^x_{11}$", "xy": (1.90, 0.69), "xytext": (1.44, 0.76)},
        {"text": r"$E^x_{12}$", "xy": (2.45, 0.43), "xytext": (2.18, 0.57)},
        {"text": r"$E^y_{21}$", "xy": (2.75, 0.56), "xytext": (2.55, 0.68)},
        {"text": r"$E^y_{12}$", "xy": (2.05, 0.28), "xytext": (1.84, 0.30)},
        {"text": r"$E^x_{21}$", "xy": (2.20, 0.33), "xytext": (1.96, 0.49)},
        {"text": r"$E^y_{31}, E^x_{13}$", "xy": (3.08, 0.35), "xytext": (2.55, 0.35)},
        {"text": r"$E^y_{13}, E^x_{31}$", "xy": (2.80, 0.14), "xytext": (2.36, 0.19)},
        {"text": r"$E^y_{22}, E^x_{22}$", "xy": (3.15, 0.46), "xytext": (3.28, 0.59)},
        {"text": r"$E^x_{32}$", "xy": (3.30, 0.09), "xytext": (3.18, 0.15)},
        {"text": r"$E^y_{23}$", "xy": (3.38, 0.10), "xytext": (3.50, 0.06)},
        {"text": r"$E^y_{32}$", "xy": (3.93, 0.25), "xytext": (3.76, 0.34)},
        {"text": r"$E^x_{23}$", "xy": (3.77, 0.18), "xytext": (3.92, 0.15)},
    ],
    "fig19": [
        {"text": r"$E^x_{11}$", "xy": (0.82, 0.31), "xytext": (0.50, 0.31)},
        {"text": r"$E^y_{11}$", "xy": (1.04, 0.44), "xytext": (0.84, 0.49)},
        {"text": r"$E^x_{21}$", "xy": (1.05, 0.56), "xytext": (0.84, 0.63)},
        {"text": r"$E^y_{21}$", "xy": (1.60, 0.62), "xytext": (1.38, 0.76)},
        {"text": r"$E^y_{31}$", "xy": (1.95, 0.60), "xytext": (2.08, 0.84)},
        {"text": r"$E^x_{31}$", "xy": (2.64, 0.63), "xytext": (2.52, 0.90)},
        {"text": r"$E^x_{12}$", "xy": (3.16, 0.73), "xytext": (3.10, 0.86)},
        {"text": r"$E^y_{12}$", "xy": (2.30, 0.30), "xytext": (2.16, 0.16)},
        {"text": r"$E^y_{22}$", "xy": (2.57, 0.38), "xytext": (2.82, 0.26)},
        {"text": r"$E^x_{22}$", "xy": (3.48, 0.56), "xytext": (3.55, 0.40)},
        {"text": r"$E^y_{41}$", "xy": (2.10, 0.33), "xytext": (2.52, 0.11)},
        {"text": r"$E^x_{41}$", "xy": (3.88, 0.66), "xytext": (3.83, 0.50)},
    ],
}


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


def add_article_annotations(ax, figure_tag):
    for item in ARTICLE_ANNOTATIONS.get(figure_tag, []):
        ax.annotate(
            item["text"],
            xy=item["xy"],
            xytext=item["xytext"],
            fontsize=9,
            ha="center",
            va="center",
            arrowprops={
                "arrowstyle": "->",
                "lw": 0.7,
                "color": "black",
                "shrinkA": 2,
                "shrinkB": 2,
            },
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
    parser.add_argument("--figure-tag", choices=sorted(ARTICLE_ANNOTATIONS))
    parser.add_argument("--no-legend", action="store_true")
    return parser.parse_args()


def main() -> None:
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
    ax.set_xticks(X_TICKS)
    ax.set_xlabel(r"$\mathcal{B}$")
    ax.set_ylabel(r"$\mathcal{P}^2$")
    ax.grid(True, alpha=0.3)

    if args.figure_tag:
        add_article_annotations(ax, args.figure_tag)

    handles, labels = ax.get_legend_handles_labels()
    if handles and not args.no_legend:
        ax.legend(frameon=False)

    ax.set_title(args.title)
    fig.tight_layout()

    if args.save:
        fig.savefig(args.save, bbox_inches="tight")
    else:
        plt.show()


if __name__ == "__main__":
    main()
