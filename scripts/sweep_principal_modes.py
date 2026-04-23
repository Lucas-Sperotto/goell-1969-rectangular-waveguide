#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
from pathlib import Path
from statistics import fmean

import matplotlib.pyplot as plt

from principal_modes import export_principal_branch
from validate_goell import (
    normalize_curve_rows,
    parse_n_values,
    run_curve_solver,
    track_stable_csv,
)


PHASES = ("phi0", "phi90")


def sanitize_value(value: float) -> str:
    text = f"{value:.6f}".rstrip("0").rstrip(".")
    return text.replace("-", "m").replace(".", "p")


def parse_float_values(raw: str) -> list[float]:
    values = []
    for token in raw.split(","):
        token = token.strip()
        if token:
            values.append(float(token))
    if not values:
        raise SystemExit("--values precisa conter pelo menos um valor")
    return values


def cluster_points(points: list[dict[str, object]], p_tol: float) -> list[list[dict[str, object]]]:
    if not points:
        return []

    points = sorted(points, key=lambda point: float(point["Pprime"]))
    clusters: list[list[dict[str, object]]] = [[points[0]]]
    for point in points[1:]:
        current = clusters[-1]
        center = fmean(float(item["Pprime"]) for item in current)
        if abs(float(point["Pprime"]) - center) <= p_tol:
            current.append(point)
        else:
            clusters.append([point])
    return clusters


def export_stable_phase(
    *,
    out_dir: Path,
    tag: str,
    param_name: str,
    param_value: float,
    phase: str,
    a_over_b: float,
    nr: float,
    n_values: list[int],
    min_support: int,
    p_tol: float,
    pscan: int,
    det_search: str,
    even_rect_mode: str,
    tracked_max_jump: float,
    tracked_min_length: int,
    tracked_monotonic_tol: float,
    tracked_min_monotonic_fraction: float,
    Bmin: float,
    Bmax: float,
    NB: int,
) -> tuple[Path, Path]:
    curves_by_n: dict[int, list[dict[str, object]]] = {}
    for N in n_values:
        raw_rows = run_curve_solver(
            parity="odd",
            phase=phase,
            geometry="intersection",
            a_over_b=a_over_b,
            nr=nr,
            N=N,
            Bmin=Bmin,
            Bmax=Bmax,
            NB=NB,
            pscan=pscan,
            metric="det",
            det_search=det_search,
            even_rect_mode=even_rect_mode,
            rescale=False,
        )
        curves_by_n[N] = normalize_curve_rows(raw_rows)

    all_B = sorted({float(row["B"]) for rows in curves_by_n.values() for row in rows})
    stable_rows: list[dict[str, object]] = []

    for B in all_B:
        points_at_B = []
        for N, rows in curves_by_n.items():
            for row in rows:
                if abs(float(row["B"]) - B) > 1e-9:
                    continue
                if not (0.001 <= float(row["Pprime"]) <= 0.95):
                    continue
                point = dict(row)
                point["N"] = N
                points_at_B.append(point)

        clusters = cluster_points(points_at_B, p_tol=p_tol)
        stable_clusters = []
        for cluster in clusters:
            support_ns = sorted({int(point["N"]) for point in cluster})
            if len(support_ns) < min_support:
                continue
            stable_clusters.append(
                {
                    "B": B,
                    "Pprime": fmean(float(point["Pprime"]) for point in cluster),
                    "merit": min(float(point["merit"]) for point in cluster),
                    "support": len(support_ns),
                    "n_values": ";".join(str(item) for item in support_ns),
                    "parity": "odd",
                    "phase": phase,
                    "geometry": "intersection",
                }
            )

        stable_clusters.sort(key=lambda cluster: float(cluster["Pprime"]))
        for branch_id, cluster in enumerate(stable_clusters):
            cluster["branch_id"] = branch_id
            stable_rows.append(cluster)

    value_tag = sanitize_value(param_value)
    stem = f"{tag}_{param_name}_{value_tag}_stable_odd_{phase}"
    stable_path = out_dir / f"{stem}.csv"
    with stable_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "branch_id",
                "B",
                "Pprime",
                "merit",
                "support",
                "n_values",
                "parity",
                "phase",
                "geometry",
            ],
        )
        writer.writeheader()
        writer.writerows(stable_rows)

    tracked_path = track_stable_csv(
        stable_path,
        max_jump=tracked_max_jump,
        min_length=tracked_min_length,
        monotonic_tol=tracked_monotonic_tol,
        min_monotonic_fraction=tracked_min_monotonic_fraction,
    )
    return stable_path, tracked_path


def draw_plot(rows: list[dict[str, str]], path: Path, *, title: str, vary: str) -> None:
    fig, ax = plt.subplots(figsize=(8.5, 5.5), dpi=160)
    values = sorted({row["param_value"] for row in rows}, key=lambda item: float(item))
    colors = plt.cm.viridis([idx / max(1, len(values) - 1) for idx in range(len(values))])
    color_map = {value: colors[idx] for idx, value in enumerate(values)}
    style_map = {"phi0": "-", "phi90": "--"}

    groups: dict[tuple[str, str], list[dict[str, str]]] = {}
    for row in rows:
        key = (row["phase"], row["param_value"])
        groups.setdefault(key, []).append(row)

    for (phase, param_value), group_rows in sorted(
        groups.items(), key=lambda item: (float(item[0][1]), item[0][0])
    ):
        group_rows.sort(key=lambda row: float(row["B"]))
        label = f"{phase}, {vary}={float(param_value):g}"
        ax.plot(
            [float(row["B"]) for row in group_rows],
            [float(row["Pprime"]) for row in group_rows],
            linestyle=style_map.get(phase, "-"),
            color=color_map[param_value],
            lw=1.4,
            alpha=0.9,
            label=label,
        )

    ax.set_xlim(0, 4)
    ax.set_ylim(0, 1)
    ax.set_xlabel("B")
    ax.set_ylabel("Pprime")
    ax.grid(True, alpha=0.3)
    ax.legend(frameon=False, fontsize=8, ncol=2)
    ax.set_title(title)
    fig.tight_layout()
    fig.savefig(path, bbox_inches="tight")
    plt.close(fig)


def write_summary(
    rows: list[dict[str, str]],
    path: Path,
    *,
    tag: str,
    vary: str,
    values: list[float],
    a_over_b: float,
    nr: float,
    even_rect_mode: str,
) -> None:
    lines = []
    lines.append(f"# Sweep Dos Modos Principais - {tag}")
    lines.append("")
    lines.append("Configuracao:")
    lines.append(f"- `vary = {vary}`")
    lines.append(f"- `values = {','.join(f'{value:g}' for value in values)}`")
    lines.append(f"- `a_over_b = {a_over_b:g}`")
    lines.append(f"- `nr = {nr:g}`")
    lines.append(f"- `even-rect-mode = {even_rect_mode}`")
    lines.append("")
    lines.append("Observacao:")
    lines.append(
        "- O par principal e selecionado como o ramo `odd/phi0` e `odd/phi90` de menor cutoff apos estabilidade em N e rastreamento em B."
    )
    lines.append("")
    lines.append("| phase | param_value | B_inicio | B_fim | P_inicio | P_fim | pontos |")
    lines.append("|:--|--:|--:|--:|--:|--:|--:|")

    groups: dict[tuple[str, str], list[dict[str, str]]] = {}
    for row in rows:
        groups.setdefault((row["phase"], row["param_value"]), []).append(row)

    for (phase, param_value), group_rows in sorted(
        groups.items(), key=lambda item: (item[0][0], float(item[0][1]))
    ):
        group_rows.sort(key=lambda row: float(row["B"]))
        lines.append(
            f"| {phase} | {float(param_value):g} | "
            f"{float(group_rows[0]['B']):.3f} | {float(group_rows[-1]['B']):.3f} | "
            f"{float(group_rows[0]['Pprime']):.6f} | {float(group_rows[-1]['Pprime']):.6f} | "
            f"{len(group_rows)} |"
        )

    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Gera sweeps dos dois modos principais a partir do pipeline estavel do Goell."
    )
    parser.add_argument("--tag", required=True, help="Prefixo dos artefatos.")
    parser.add_argument("--vary", required=True, choices=("nr", "a_over_b"))
    parser.add_argument("--values", required=True, help="Lista CSV dos valores varridos.")
    parser.add_argument("--a-over-b", type=float, default=1.0)
    parser.add_argument("--nr", type=float, default=1.0001)
    parser.add_argument("--n-values", default="5,7,9")
    parser.add_argument("--min-support", type=int, default=2)
    parser.add_argument("--p-tol", type=float, default=0.03)
    parser.add_argument("--pscan", type=int, default=240)
    parser.add_argument("--det-search", default="sign", choices=("minima", "sign"))
    parser.add_argument(
        "--even-rect-mode",
        default="paper",
        choices=("paper", "square-split"),
    )
    parser.add_argument("--tracked-max-jump", type=float, default=0.12)
    parser.add_argument("--tracked-min-length", type=int, default=6)
    parser.add_argument("--tracked-monotonic-tol", type=float, default=0.02)
    parser.add_argument("--tracked-min-monotonic-fraction", type=float, default=0.85)
    parser.add_argument("--Bmin", type=float, default=0.0)
    parser.add_argument("--Bmax", type=float, default=4.0)
    parser.add_argument("--NB", type=int, default=40)
    parser.add_argument("--title", default="Goell CHM - sweep dos modos principais")
    parser.add_argument(
        "--reuse-existing",
        action="store_true",
        help="Reusa CSVs *_principal.csv ja existentes e calcula apenas os valores faltantes.",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    values = parse_float_values(args.values)
    n_values = parse_n_values(args.n_values)

    out_dir = Path("out")
    fig_dir = Path("figures")
    out_dir.mkdir(exist_ok=True)
    fig_dir.mkdir(exist_ok=True)

    aggregate_rows: list[dict[str, str]] = []
    for value in values:
        a_over_b = value if args.vary == "a_over_b" else args.a_over_b
        nr = value if args.vary == "nr" else args.nr

        for phase in PHASES:
            value_tag = sanitize_value(value)
            principal_path = out_dir / (
                f"{args.tag}_{args.vary}_{value_tag}_stable_odd_{phase}_tracked_principal.csv"
            )
            if not (args.reuse_existing and principal_path.exists()):
                _stable_path, tracked_path = export_stable_phase(
                    out_dir=out_dir,
                    tag=args.tag,
                    param_name=args.vary,
                    param_value=value,
                    phase=phase,
                    a_over_b=a_over_b,
                    nr=nr,
                    n_values=n_values,
                    min_support=args.min_support,
                    p_tol=args.p_tol,
                    pscan=args.pscan,
                    det_search=args.det_search,
                    even_rect_mode=args.even_rect_mode,
                    tracked_max_jump=args.tracked_max_jump,
                    tracked_min_length=args.tracked_min_length,
                    tracked_monotonic_tol=args.tracked_monotonic_tol,
                    tracked_min_monotonic_fraction=args.tracked_min_monotonic_fraction,
                    Bmin=args.Bmin,
                    Bmax=args.Bmax,
                    NB=args.NB,
                )
                principal_path = tracked_path.with_name(f"{tracked_path.stem}_principal.csv")
                export_principal_branch(
                    tracked_path,
                    principal_path,
                    curve_label=f"{args.tag}_{phase}",
                    param_name=args.vary,
                    param_value=f"{value:.6f}",
                    mode_alias=f"principal_{phase}",
                )
            with principal_path.open(newline="", encoding="utf-8") as handle:
                aggregate_rows.extend(list(csv.DictReader(handle)))

    aggregate_path = out_dir / f"{args.tag}_principal_modes.csv"
    fieldnames = [
        "branch_id",
        "B",
        "Pprime",
        "merit",
        "branch_len",
        "parity",
        "phase",
        "geometry",
        "curve_label",
        "param_name",
        "param_value",
        "mode_alias",
    ]
    with aggregate_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(aggregate_rows)

    figure_path = fig_dir / f"{args.tag}_principal_modes.png"
    draw_plot(aggregate_rows, figure_path, title=args.title, vary=args.vary)

    summary_path = out_dir / f"{args.tag}_principal_modes.md"
    write_summary(
        aggregate_rows,
        summary_path,
        tag=args.tag,
        vary=args.vary,
        values=values,
        a_over_b=args.a_over_b,
        nr=args.nr,
        even_rect_mode=args.even_rect_mode,
    )

    print(f"CSV: {aggregate_path}")
    print(f"Markdown: {summary_path}")
    print(f"Imagem: {figure_path}")


if __name__ == "__main__":
    main()
