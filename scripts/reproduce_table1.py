#!/usr/bin/env python3
"""
Reproduz a Tabela I do paper do Goell e gera saidas em:
- tela (stdout)
- CSV
- Markdown
- imagem PNG

Modos de selecao:
- autonomous: escolhe o cluster impar de maior P' em B=2, sem usar o valor do paper
- nearest-paper: modo diagnostico legado; escolhe a raiz mais proxima do valor publicado
"""

from __future__ import annotations

import argparse
import csv
import math
import sys
from pathlib import Path

import matplotlib.pyplot as plt

from solver_api import call_solver, ensure_solver_exists


TABLE1_EXPECTED = {
    1.0: {3: 0.714, 4: 0.713, 5: 0.715, 6: 0.714, 7: 0.715, 8: 0.715, 9: 0.715},
    2.0: {3: 0.811, 4: 0.811, 5: 0.808, 6: 0.808, 7: 0.808, 8: 0.807, 9: 0.807},
    3.0: {3: 0.820, 4: 0.820, 5: 0.819, 6: 0.822, 7: 0.820, 8: 0.820, 9: 0.823},
    4.0: {3: 0.828, 4: 0.819, 5: 0.813, 6: 0.820, 7: 0.813, 8: 0.814, 9: 0.815},
}

AB_VALUES = [1.0, 2.0, 3.0, 4.0]
N_VALUES = [3, 4, 5, 6, 7, 8, 9]
ODD_CLUSTER_FLOOR = 0.6
CLUSTER_TOL = 0.03


def run_solver_case(
    *,
    a_over_b: float,
    N: int,
    pscan: int,
    even_rect_mode: str,
) -> list[dict[str, object]]:
    candidates: list[dict[str, object]] = []
    for parity in ("odd", "even"):
        for phase in ("phi0", "phi90"):
            rows = call_solver([
                "--parity", parity,
                "--phase", phase,
                "--geometry", "intersection",
                "--a_over_b", str(a_over_b),
                "--nr", "1.01",
                "--N", str(N),
                "--Bmin", "2", "--Bmax", "2", "--NB", "0",
                "--Pscan", str(pscan),
                "--metric", "det",
                "--det-search", "sign",
                "--even-rect-mode", even_rect_mode,
                "--all-minima", "--no-rescale",
            ])
            for row in rows:
                pprime = float(row["Pprime"])
                merit = float(row["merit"])
                if 0.0 < pprime < 1.0 and math.isfinite(pprime):
                    candidates.append(
                        {
                            "Pprime": pprime,
                            "merit": merit,
                            "parity": parity,
                            "phase": phase,
                        }
                    )
    return sorted(candidates, key=lambda item: float(item["Pprime"]))


def average_cluster(points: list[dict[str, object]]) -> dict[str, object]:
    phases = sorted({str(point["phase"]) for point in points})
    phase_label = "+".join(phases) if len(phases) > 1 else phases[0]
    return {
        "solver": sum(float(point["Pprime"]) for point in points) / len(points),
        "merit": min(float(point["merit"]) for point in points),
        "parity": str(points[0]["parity"]),
        "phase": phase_label,
        "support": len(points),
    }


def select_autonomous_first_mode(candidates: list[dict[str, object]]) -> dict[str, object]:
    odd_candidates = [
        candidate
        for candidate in candidates
        if str(candidate["parity"]) == "odd" and float(candidate["Pprime"]) >= ODD_CLUSTER_FLOOR
    ]
    if not odd_candidates:
        odd_candidates = [
            candidate for candidate in candidates if str(candidate["parity"]) == "odd"
        ]
    if not odd_candidates:
        raise ValueError("nao foi encontrado nenhum candidato impar para a Tabela I")

    cluster = [odd_candidates[-1]]
    for candidate in reversed(odd_candidates[:-1]):
        center = sum(float(point["Pprime"]) for point in cluster) / len(cluster)
        if abs(float(candidate["Pprime"]) - center) <= CLUSTER_TOL:
            cluster.append(candidate)
        else:
            break

    picked = average_cluster(sorted(cluster, key=lambda item: float(item["Pprime"])))
    picked["selector"] = "highest_odd_cluster"
    return picked


def select_nearest_paper(
    candidates: list[dict[str, object]],
    *,
    expected: float,
) -> dict[str, object]:
    picked = min(candidates, key=lambda item: abs(float(item["Pprime"]) - expected))
    return {
        "solver": float(picked["Pprime"]),
        "merit": float(picked["merit"]),
        "parity": str(picked["parity"]),
        "phase": str(picked["phase"]),
        "support": 1,
        "selector": "nearest_paper",
    }


def select_candidate(
    candidates: list[dict[str, object]],
    *,
    mode: str,
    expected: float,
) -> dict[str, object]:
    if mode == "autonomous":
        return select_autonomous_first_mode(candidates)
    if mode == "nearest-paper":
        return select_nearest_paper(candidates, expected=expected)
    raise ValueError(f"modo invalido: {mode}")


def reproduce_table(*, pscan: int, mode: str, even_rect_mode: str) -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    for N in N_VALUES:
        for a_over_b in AB_VALUES:
            expected = TABLE1_EXPECTED[a_over_b][N]
            candidates = run_solver_case(
                a_over_b=a_over_b,
                N=N,
                pscan=pscan,
                even_rect_mode=even_rect_mode,
            )
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
                        "support": 0,
                        "selector": mode,
                    }
                )
                continue

            picked = select_candidate(candidates, mode=mode, expected=expected)
            rows.append(
                {
                    "N": N,
                    "a_over_b": a_over_b,
                    "paper": expected,
                    "solver": float(picked["solver"]),
                    "error": float(picked["solver"]) - expected,
                    "parity": str(picked["parity"]),
                    "phase": str(picked["phase"]),
                    "merit": float(picked["merit"]),
                    "support": int(picked["support"]),
                    "selector": str(picked["selector"]),
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


def error_metrics(rows: list[dict[str, object]]) -> tuple[float, float, float]:
    all_errors = [float(row["error"]) for row in rows if math.isfinite(float(row["error"]))]
    rmse = (sum(error * error for error in all_errors) / len(all_errors)) ** 0.5
    mae = sum(abs(error) for error in all_errors) / len(all_errors)
    worst = max(abs(error) for error in all_errors)
    return rmse, mae, worst


def selection_note(mode: str) -> str:
    if mode == "autonomous":
        return (
            "Modo autonomo: escolhe o cluster impar de maior P' em B=2, "
            "sem usar o valor publicado como referencia."
        )
    return (
        "Modo diagnostico: escolhe a raiz mais proxima do valor publicado no paper."
    )


def write_csv(rows: list[dict[str, object]], path: Path) -> None:
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "N",
                "a_over_b",
                "paper",
                "solver",
                "error",
                "parity",
                "phase",
                "merit",
                "support",
                "selector",
            ],
        )
        writer.writeheader()
        writer.writerows(rows)


def write_markdown(
    rows: list[dict[str, object]],
    path: Path,
    *,
    pscan: int,
    mode: str,
    even_rect_mode: str,
) -> None:
    lines = []
    lines.append("# Reproducao Da Tabela I")
    lines.append("")
    lines.append("Configuracao numerica usada:")
    lines.append("- `geometry = intersection`")
    lines.append("- `metric = det`")
    lines.append("- `det-search = sign`")
    lines.append(f"- `even-rect-mode = {even_rect_mode}`")
    lines.append("- `no-rescale`")
    lines.append("- `n_r = 1.01`")
    lines.append("- `B = 2`")
    lines.append(f"- `Pscan = {pscan}`")
    lines.append(f"- `selection_mode = {mode}`")
    lines.append("")
    lines.append("Observacao:")
    lines.append(selection_note(mode))
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
    lines.append("| N | a/b | paper | solver | erro | classe | support | selector | merit |")
    lines.append("|---:|---:|---:|---:|---:|:---|---:|:---|---:|")
    for row in rows:
        lines.append(
            f"| {row['N']} | {row['a_over_b']:.0f} | {row['paper']:.3f} | {row['solver']:.6f} | "
            f"{row['error']:+.6f} | {row['parity']}/{row['phase']} | {row['support']} | "
            f"{row['selector']} | {row['merit']:.6f} |"
        )
    rmse, mae, worst = error_metrics(rows)
    lines.append("")
    lines.append("## Metricas")
    lines.append("")
    lines.append(f"- `RMSE = {rmse:.6f}`")
    lines.append(f"- `MAE = {mae:.6f}`")
    lines.append(f"- `Pior erro absoluto = {worst:.6f}`")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def draw_table_image(
    rows: list[dict[str, object]],
    path: Path,
    *,
    pscan: int,
    mode: str,
    even_rect_mode: str,
) -> None:
    col_labels = ["N", "a/b=1", "a/b=2", "a/b=3", "a/b=4"]
    cell_text = []

    for N in N_VALUES:
        row = [str(N)]
        for a_over_b in AB_VALUES:
            solver = solver_value(rows, N, a_over_b)
            error = solver - paper_value(N, a_over_b)
            row.append(f"{solver:.3f}\n({error:+.3f})")
        cell_text.append(row)

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
        f"even-rect-mode={even_rect_mode}, no-rescale, nr=1.01, B=2, Pscan={pscan}\n"
        f"Selecao: {selection_note(mode)}"
    )
    fig.text(0.5, 0.03, footer, ha="center", va="bottom", fontsize=8)
    fig.tight_layout(rect=(0.02, 0.08, 0.98, 0.95))
    fig.savefig(path, bbox_inches="tight")
    plt.close(fig)


def print_stdout_summary(
    rows: list[dict[str, object]],
    *,
    pscan: int,
    mode: str,
    even_rect_mode: str,
) -> None:
    print("Reproducao da Tabela I")
    print(
        "Configuracao: geometry=intersection, metric=det, det-search=sign, "
        f"even-rect-mode={even_rect_mode}, no-rescale, nr=1.01, B=2"
    )
    print(f"Pscan = {pscan}")
    print(f"Selecao atual: {selection_note(mode)}")
    print()

    header = "N   a/b=1      a/b=2      a/b=3      a/b=4"
    print(header)
    for N in N_VALUES:
        values = [solver_value(rows, N, a_over_b) for a_over_b in AB_VALUES]
        print(f"{N:<2}  " + "  ".join(f"{value:0.6f}" for value in values))
    print()
    print("Erros solver - paper")
    print(header)
    for N in N_VALUES:
        errors = [solver_value(rows, N, a_over_b) - paper_value(N, a_over_b) for a_over_b in AB_VALUES]
        print(f"{N:<2}  " + "  ".join(f"{error:+0.6f}" for error in errors))
    print()

    rmse, mae, worst = error_metrics(rows)
    print(f"RMSE = {rmse:.6f}")
    print(f"MAE  = {mae:.6f}")
    print(f"Pior erro absoluto = {worst:.6f}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Reproduz a Tabela I do paper do Goell.")
    parser.add_argument(
        "--mode",
        default="autonomous",
        choices=("autonomous", "nearest-paper"),
        help="Modo de selecao modal: autonomo ou diagnostico legado.",
    )
    parser.add_argument("--pscan", type=int, default=240, help="Numero de amostras em P'.")
    parser.add_argument(
        "--even-rect-mode",
        default="paper",
        choices=("paper", "square-split"),
        help="Regra usada no caso even com a/b != 1.",
    )
    parser.add_argument(
        "--mae-threshold",
        type=float,
        default=0.01,
        help="Threshold de MAE usado com --fail-on-threshold.",
    )
    parser.add_argument(
        "--max-error-threshold",
        type=float,
        default=0.02,
        help="Threshold de pior erro absoluto usado com --fail-on-threshold.",
    )
    parser.add_argument(
        "--fail-on-threshold",
        action="store_true",
        help="Sai com codigo 1 se MAE ou pior erro ultrapassarem os thresholds.",
    )
    return parser.parse_args()


def main() -> None:
    ensure_solver_exists()
    args = parse_args()

    out_dir = Path("out")
    fig_dir = Path("figures")
    out_dir.mkdir(exist_ok=True)
    fig_dir.mkdir(exist_ok=True)

    rows = reproduce_table(
        pscan=args.pscan,
        mode=args.mode,
        even_rect_mode=args.even_rect_mode,
    )
    prefix = f"table1_reproduced_{args.mode.replace('-', '_')}"
    if args.even_rect_mode != "paper":
        prefix += f"_even_rect_{args.even_rect_mode.replace('-', '_')}"

    csv_path = out_dir / f"{prefix}.csv"
    md_path = out_dir / f"{prefix}.md"
    png_path = fig_dir / f"{prefix}.png"

    write_csv(rows, csv_path)
    write_markdown(
        rows,
        md_path,
        pscan=args.pscan,
        mode=args.mode,
        even_rect_mode=args.even_rect_mode,
    )
    draw_table_image(
        rows,
        png_path,
        pscan=args.pscan,
        mode=args.mode,
        even_rect_mode=args.even_rect_mode,
    )
    print_stdout_summary(
        rows,
        pscan=args.pscan,
        mode=args.mode,
        even_rect_mode=args.even_rect_mode,
    )
    print()
    print(f"CSV: {csv_path}")
    print(f"Markdown: {md_path}")
    print(f"Imagem: {png_path}")

    if args.fail_on_threshold:
        _rmse, mae, worst = error_metrics(rows)
        if mae > args.mae_threshold or worst > args.max_error_threshold:
            print()
            print(
                "Falha nos thresholds: "
                f"MAE={mae:.6f} (limite {args.mae_threshold:.6f}), "
                f"pior={worst:.6f} (limite {args.max_error_threshold:.6f})",
                file=sys.stderr,
            )
            raise SystemExit(1)


if __name__ == "__main__":
    main()
