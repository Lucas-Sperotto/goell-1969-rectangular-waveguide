#!/usr/bin/env python3
"""
Reproduz a Tabela I do paper do Goell e gera saidas em:
- tela (stdout)
- CSV
- Markdown
- imagem PNG

Observacao importante:
esta rotina ainda nao identifica o "primeiro modo" de forma independente
apenas pela fisica do problema. Para cada caso, ela escolhe a raiz calculada
mais proxima do valor publicado no paper. Ou seja: hoje ela serve como uma
ferramenta de conferencia numerica, nao como prova final de identificacao modal.
"""

from __future__ import annotations

import csv
import io
import math
import subprocess
from pathlib import Path

import matplotlib.pyplot as plt


TABLE1_EXPECTED = {
    1.0: {3: 0.714, 4: 0.713, 5: 0.715, 6: 0.714, 7: 0.715, 8: 0.715, 9: 0.715},
    2.0: {3: 0.811, 4: 0.811, 5: 0.808, 6: 0.808, 7: 0.808, 8: 0.807, 9: 0.807},
    3.0: {3: 0.820, 4: 0.820, 5: 0.819, 6: 0.822, 7: 0.820, 8: 0.820, 9: 0.823},
    4.0: {3: 0.828, 4: 0.819, 5: 0.813, 6: 0.820, 7: 0.813, 8: 0.814, 9: 0.815},
}

AB_VALUES = [1.0, 2.0, 3.0, 4.0]
N_VALUES = [3, 4, 5, 6, 7, 8, 9]


def run_solver_case(*, a_over_b: float, N: int, pscan: int) -> list[tuple[float, float, str, str]]:
    candidates: list[tuple[float, float, str, str]] = []
    for parity in ("odd", "even"):
        for phase in ("phi0", "phi90"):
            cmd = [
                "./build/goell_q_solver",
                "--parity",
                parity,
                "--phase",
                phase,
                "--geometry",
                "intersection",
                "--a_over_b",
                str(a_over_b),
                "--nr",
                "1.01",
                "--N",
                str(N),
                "--Bmin",
                "2",
                "--Bmax",
                "2",
                "--NB",
                "0",
                "--Pscan",
                str(pscan),
                "--metric",
                "det",
                "--det-search",
                "sign",
                "--all-minima",
                "--no-rescale",
            ]
            out = subprocess.check_output(cmd, text=True)
            rows = list(csv.DictReader(io.StringIO(out)))
            for row in rows:
                pprime = float(row["Pprime"])
                merit = float(row["merit"])
                if 0.0 < pprime < 1.0 and math.isfinite(pprime):
                    candidates.append((pprime, merit, parity, phase))
    return candidates


def reproduce_table(pscan: int) -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    for N in N_VALUES:
        for a_over_b in AB_VALUES:
            expected = TABLE1_EXPECTED[a_over_b][N]
            candidates = run_solver_case(a_over_b=a_over_b, N=N, pscan=pscan)
            if not candidates:
                rows.append(
                    {
                        "N": N,
                        "a_over_b": a_over_b,
                        "paper": expected,
                        "solver": float("nan"),
                        "error": float("nan"),
                        "parity": "",
                        "phase": "",
                        "merit": float("nan"),
                    }
                )
                continue

            pprime, merit, parity, phase = min(candidates, key=lambda item: abs(item[0] - expected))
            rows.append(
                {
                    "N": N,
                    "a_over_b": a_over_b,
                    "paper": expected,
                    "solver": pprime,
                    "error": pprime - expected,
                    "parity": parity,
                    "phase": phase,
                    "merit": merit,
                }
            )
    return rows


def solver_value(rows: list[dict[str, object]], N: int, a_over_b: float) -> float:
    for row in rows:
        if row["N"] == N and abs(float(row["a_over_b"]) - a_over_b) < 1e-12:
            return float(row["solver"])
    return float("nan")


def paper_value(N: int, a_over_b: float) -> float:
    return TABLE1_EXPECTED[a_over_b][N]


def variation_percent(values: list[float]) -> float:
    finite = [value for value in values if math.isfinite(value)]
    if not finite:
        return float("nan")
    return 100.0 * (max(finite) - min(finite))


def write_csv(rows: list[dict[str, object]], path: Path) -> None:
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=["N", "a_over_b", "paper", "solver", "error", "parity", "phase", "merit"],
        )
        writer.writeheader()
        writer.writerows(rows)


def write_markdown(rows: list[dict[str, object]], path: Path, pscan: int) -> None:
    lines = []
    lines.append("# Reproducao Da Tabela I")
    lines.append("")
    lines.append("Configuracao numerica usada:")
    lines.append("- `geometry = intersection`")
    lines.append("- `metric = det`")
    lines.append("- `det-search = sign`")
    lines.append("- `no-rescale`")
    lines.append("- `n_r = 1.01`")
    lines.append("- `B = 2`")
    lines.append(f"- `Pscan = {pscan}`")
    lines.append("")
    lines.append("Observacao:")
    lines.append("A raiz escolhida em cada caso ainda e a mais proxima do valor publicado no paper.")
    lines.append("")
    lines.append("| N | a/b=1 paper | a/b=1 solver | a/b=2 paper | a/b=2 solver | a/b=3 paper | a/b=3 solver | a/b=4 paper | a/b=4 solver |")
    lines.append("|---:|---:|---:|---:|---:|---:|---:|---:|---:|")
    for N in N_VALUES:
        parts = [f"| {N} "]
        for a_over_b in AB_VALUES:
            parts.append(f"| {paper_value(N, a_over_b):.3f} ")
            parts.append(f"| {solver_value(rows, N, a_over_b):.6f} ")
        parts.append("|")
        lines.append("".join(parts))
    lines.append("")
    lines.append("| a/b | variacao paper | variacao solver |")
    lines.append("|---:|---:|---:|")
    for a_over_b in AB_VALUES:
        paper_var = variation_percent([paper_value(N, a_over_b) for N in N_VALUES])
        solver_var = variation_percent([solver_value(rows, N, a_over_b) for N in N_VALUES])
        lines.append(f"| {a_over_b:g} | {paper_var:.1f}% | {solver_var:.1f}% |")
    lines.append("")
    lines.append("## Detalhes")
    lines.append("")
    lines.append("| N | a/b | paper | solver | erro | classe | merit |")
    lines.append("|---:|---:|---:|---:|---:|:---|---:|")
    for row in rows:
        lines.append(
            f"| {row['N']} | {row['a_over_b']:.0f} | {row['paper']:.3f} | {row['solver']:.6f} | "
            f"{row['error']:+.6f} | {row['parity']}/{row['phase']} | {row['merit']:.6f} |"
        )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def draw_table_image(rows: list[dict[str, object]], path: Path, pscan: int) -> None:
    col_labels = ["N", "a/b=1", "a/b=2", "a/b=3", "a/b=4"]
    cell_text = []
    row_labels = []

    for N in N_VALUES:
        row_labels.append(str(N))
        row = [str(N)]
        for a_over_b in AB_VALUES:
            solver = solver_value(rows, N, a_over_b)
            error = solver - paper_value(N, a_over_b)
            row.append(f"{solver:.3f}\n({error:+.3f})")
        cell_text.append(row)

    row_labels.append("Var.")
    variation_row = ["Var."]
    for a_over_b in AB_VALUES:
        solver_var = variation_percent([solver_value(rows, N, a_over_b) for N in N_VALUES])
        variation_row.append(f"{solver_var:.1f}%")
    cell_text.append(variation_row)

    fig, ax = plt.subplots(figsize=(8.5, 5.5), dpi=180)
    ax.axis("off")

    title = (
        "Tabela I reproduzida pelo solver\n"
        "valores calculados; erro entre parenteses"
    )
    ax.set_title(title, fontsize=12, pad=12)

    table = ax.table(
        cellText=cell_text,
        colLabels=col_labels,
        cellLoc="center",
        loc="center",
    )
    table.auto_set_font_size(False)
    table.set_fontsize(9)
    table.scale(1.15, 1.6)

    footer = (
        "Configuracao: geometry=intersection, metric=det, det-search=sign, "
        f"no-rescale, nr=1.01, B=2, Pscan={pscan}\n"
        "Selecao atual: raiz calculada mais proxima do valor publicado."
    )
    fig.text(0.5, 0.03, footer, ha="center", va="bottom", fontsize=8)
    fig.tight_layout(rect=(0.02, 0.08, 0.98, 0.95))
    fig.savefig(path, bbox_inches="tight")
    plt.close(fig)


def print_stdout_summary(rows: list[dict[str, object]], pscan: int) -> None:
    print("Reproducao da Tabela I")
    print("Configuracao: geometry=intersection, metric=det, det-search=sign, no-rescale, nr=1.01, B=2")
    print(f"Pscan = {pscan}")
    print("Selecao atual: raiz calculada mais proxima do valor publicado no paper.")
    print()

    header = "N   a/b=1      a/b=2      a/b=3      a/b=4"
    print(header)
    for N in N_VALUES:
        values = [solver_value(rows, N, a_over_b) for a_over_b in AB_VALUES]
        print(
            f"{N:<2}  "
            + "  ".join(f"{value:0.6f}" for value in values)
        )
    print()
    print("Erros solver - paper")
    print(header)
    for N in N_VALUES:
        errors = [solver_value(rows, N, a_over_b) - paper_value(N, a_over_b) for a_over_b in AB_VALUES]
        print(
            f"{N:<2}  "
            + "  ".join(f"{error:+0.6f}" for error in errors)
        )
    print()

    all_errors = [float(row["error"]) for row in rows if math.isfinite(float(row["error"]))]
    rmse = (sum(error * error for error in all_errors) / len(all_errors)) ** 0.5
    mae = sum(abs(error) for error in all_errors) / len(all_errors)
    worst = max(abs(error) for error in all_errors)
    print(f"RMSE = {rmse:.6f}")
    print(f"MAE  = {mae:.6f}")
    print(f"Pior erro absoluto = {worst:.6f}")


def main() -> None:
    out_dir = Path("out")
    fig_dir = Path("figures")
    out_dir.mkdir(exist_ok=True)
    fig_dir.mkdir(exist_ok=True)

    pscan = 240
    rows = reproduce_table(pscan=pscan)

    csv_path = out_dir / "table1_reproduced_sign.csv"
    md_path = out_dir / "table1_reproduced_sign.md"
    png_path = fig_dir / "table1_reproduced_sign.png"

    write_csv(rows, csv_path)
    write_markdown(rows, md_path, pscan=pscan)
    draw_table_image(rows, png_path, pscan=pscan)
    print_stdout_summary(rows, pscan=pscan)
    print()
    print(f"CSV: {csv_path}")
    print(f"Markdown: {md_path}")
    print(f"Imagem: {png_path}")


if __name__ == "__main__":
    main()
