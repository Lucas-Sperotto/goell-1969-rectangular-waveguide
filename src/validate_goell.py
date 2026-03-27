#!/usr/bin/env python3
"""
Ferramentas de validacao para a reproducao do paper do Goell.

Objetivos desta versao:
- comparar a Tabela I com os minimos encontrados em B = 2;
- resumir rapidamente as saidas CSV das figuras 16-19;
- filtrar, de forma reprodutivel, as raizes que persistem ao variar N.

O filtro de estabilidade em N nao substitui um branch tracking fisico final.
Ele existe para separar melhor:
- raizes que aparecem de forma consistente quando o truncamento muda; e
- raizes espurias que surgem apenas em um N isolado.
"""

from __future__ import annotations

import argparse
import csv
import io
import subprocess
from pathlib import Path
from statistics import fmean


TABLE1_EXPECTED = {
    1.0: {3: 0.714, 4: 0.713, 5: 0.715, 6: 0.714, 7: 0.715, 8: 0.715, 9: 0.715},
    2.0: {3: 0.811, 4: 0.811, 5: 0.808, 6: 0.808, 7: 0.808, 8: 0.807, 9: 0.807},
    3.0: {3: 0.820, 4: 0.820, 5: 0.819, 6: 0.822, 7: 0.820, 8: 0.820, 9: 0.823},
    4.0: {3: 0.828, 4: 0.819, 5: 0.813, 6: 0.820, 7: 0.813, 8: 0.814, 9: 0.815},
}

CLASS_CASES = (
    ("odd", "phi0", "odd_phi0"),
    ("odd", "phi90", "odd_phi90"),
    ("even", "phi0", "even_phi0"),
    ("even", "phi90", "even_phi90"),
)

# Para as figuras com Delta n_r -> 0, usar n_r exatamente igual a 1.0 tende a
# degenerar a normalizacao numerica. O helper de validacao aproxima esse limite
# com 1.0001 por padrao.
FIGURE_DEFAULTS = {
    "fig16": {"a_over_b": 1.0, "nr": 1.0001, "Bmin": 0.0, "Bmax": 4.0, "NB": 40},
    "fig17": {"a_over_b": 2.0, "nr": 1.0001, "Bmin": 0.0, "Bmax": 4.0, "NB": 40},
    "fig18": {"a_over_b": 1.0, "nr": 1.5, "Bmin": 0.0, "Bmax": 4.0, "NB": 40},
    "fig19": {"a_over_b": 2.0, "nr": 1.5, "Bmin": 0.0, "Bmax": 4.0, "NB": 40},
}


def run_solver(
    *,
    parity: str,
    phase: str,
    geometry: str,
    a_over_b: float,
    nr: float,
    N: int,
    B: float,
    pscan: int,
    metric: str,
    det_search: str,
    rescale: bool,
) -> list[dict[str, str]]:
    cmd = [
        "./build/goell_q_solver",
        "--parity",
        parity,
        "--phase",
        phase,
        "--geometry",
        geometry,
        "--a_over_b",
        str(a_over_b),
        "--nr",
        str(nr),
        "--N",
        str(N),
        "--Bmin",
        str(B),
        "--Bmax",
        str(B),
        "--NB",
        "0",
        "--Pscan",
        str(pscan),
        "--metric",
        metric,
        "--det-search",
        det_search,
        "--dump-scan",
        str(B),
    ]
    if not rescale:
        cmd.append("--no-rescale")

    out = subprocess.check_output(cmd, text=True)
    return list(csv.DictReader(io.StringIO(out)))


def run_curve_solver(
    *,
    parity: str,
    phase: str,
    geometry: str,
    a_over_b: float,
    nr: float,
    N: int,
    Bmin: float,
    Bmax: float,
    NB: int,
    pscan: int,
    metric: str,
    det_search: str,
    rescale: bool,
) -> list[dict[str, str]]:
    cmd = [
        "./build/goell_q_solver",
        "--parity",
        parity,
        "--phase",
        phase,
        "--geometry",
        geometry,
        "--a_over_b",
        str(a_over_b),
        "--nr",
        str(nr),
        "--N",
        str(N),
        "--Bmin",
        str(Bmin),
        "--Bmax",
        str(Bmax),
        "--NB",
        str(NB),
        "--Pscan",
        str(pscan),
        "--metric",
        metric,
        "--det-search",
        det_search,
        "--all-minima",
    ]
    if not rescale:
        cmd.append("--no-rescale")

    out = subprocess.check_output(cmd, text=True)
    return list(csv.DictReader(io.StringIO(out)))


def parse_n_values(raw: str) -> list[int]:
    values = []
    for token in raw.split(","):
        token = token.strip()
        if token:
            values.append(int(token))
    if not values:
        raise SystemExit("--n-values precisa conter pelo menos um N")
    return values


def normalize_curve_rows(rows: list[dict[str, str]]) -> list[dict[str, object]]:
    out = []
    for row in rows:
        pprime = float(row["Pprime"])
        merit_raw = row.get("merit", "").strip()
        merit = float(merit_raw) if merit_raw else float("nan")
        out.append(
            {
                "branch_id": int(row["branch_id"]),
                "B": float(row["B"]),
                "Pprime": pprime,
                "merit": merit,
                "parity": row.get("parity", "").strip(),
                "phase": row.get("phase", "").strip(),
                "geometry": row.get("geometry", "").strip(),
            }
        )
    return out


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


def export_stable_figure(
    *,
    tag: str,
    n_values: list[int],
    min_support: int,
    p_tol: float,
    pscan: int,
    metric: str,
    det_search: str,
    geometry: str,
    rescale: bool,
) -> list[Path]:
    if tag not in FIGURE_DEFAULTS:
        raise SystemExit(f"Figura desconhecida para estabilidade: {tag}")

    cfg = FIGURE_DEFAULTS[tag]
    out_dir = Path("out")
    out_dir.mkdir(exist_ok=True)
    exported: list[Path] = []

    print(
        f"Filtro de estabilidade para {tag}: "
        f"a/b={cfg['a_over_b']}, n_r={cfg['nr']}, Ns={n_values}, "
        f"support>={min_support}, p_tol={p_tol:.3f}"
    )

    for parity, phase, suffix in CLASS_CASES:
        curves_by_n: dict[int, list[dict[str, object]]] = {}
        for N in n_values:
            raw_rows = run_curve_solver(
                parity=parity,
                phase=phase,
                geometry=geometry,
                a_over_b=cfg["a_over_b"],
                nr=cfg["nr"],
                N=N,
                Bmin=cfg["Bmin"],
                Bmax=cfg["Bmax"],
                NB=cfg["NB"],
                pscan=pscan,
                metric=metric,
                det_search=det_search,
                rescale=rescale,
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
                        "parity": parity,
                        "phase": phase,
                        "geometry": geometry,
                        "support": len(support_ns),
                        "n_values": ";".join(str(N) for N in support_ns),
                    }
                )

            stable_clusters.sort(key=lambda cluster: float(cluster["Pprime"]))
            for branch_id, cluster in enumerate(stable_clusters):
                cluster["branch_id"] = branch_id
                stable_rows.append(cluster)

        out_path = out_dir / f"{tag}_stable_{suffix}.csv"
        with out_path.open("w", newline="", encoding="utf-8") as handle:
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

        exported.append(out_path)

        if stable_rows:
            bmin = min(float(row["B"]) for row in stable_rows)
            bmax = max(float(row["B"]) for row in stable_rows)
            pmin = min(float(row["Pprime"]) for row in stable_rows)
            pmax = max(float(row["Pprime"]) for row in stable_rows)
            print(
                f"  {out_path.name}: n={len(stable_rows)} "
                f"B=[{bmin:.3f},{bmax:.3f}] P=[{pmin:.3f},{pmax:.3f}]"
            )
        else:
            print(f"  {out_path.name}: sem clusters estaveis")

    print()
    return exported


def local_minima(rows: list[dict[str, str]]) -> list[tuple[float, float]]:
    vals = [(float(r["Pprime"]), float(r["merit"])) for r in rows]
    mins = []
    for i, (p, m) in enumerate(vals):
        if 0 < i < len(vals) - 1 and m < vals[i - 1][1] and m < vals[i + 1][1]:
            mins.append((p, m))
    return mins


def compare_table1(pscan: int) -> None:
    print("Comparacao com a Tabela I")
    print("Configuracao: geometry=intersection, metric=det, no-rescale, nr=1.01, B=2")
    print()

    for a_over_b in [1.0, 2.0, 3.0, 4.0]:
        print(f"a/b = {a_over_b:g}")
        for N, expected in TABLE1_EXPECTED[a_over_b].items():
            minima = []
            for parity in ("odd", "even"):
                for phase in ("phi0", "phi90"):
                    rows = run_solver(
                        parity=parity,
                        phase=phase,
                        geometry="intersection",
                        a_over_b=a_over_b,
                        nr=1.01,
                        N=N,
                        B=2.0,
                        pscan=pscan,
                        metric="det",
                        det_search="minima",
                        rescale=False,
                    )
                    for p, m in local_minima(rows):
                        minima.append((abs(p - expected), p, m, parity, phase))

            minima.sort()
            if not minima:
                print(f"  N={N}: sem minimos internos")
                continue

            _, p, m, parity, phase = minima[0]
            err = p - expected
            print(
                f"  N={N}: esperado={expected:.3f} achado={p:.6f} "
                f"erro={err:+.6f} classe={parity}/{phase} merit={m:.6f}"
            )
        print()


def summarize_figure(tag: str) -> None:
    print(f"Resumo de {tag}")
    for suffix in ("odd_phi0", "odd_phi90", "even_phi0", "even_phi90"):
        path = Path("out") / f"{tag}_{suffix}.csv"
        if not path.exists():
            print(f"  {path.name}: arquivo ausente")
            continue

        rows = []
        with path.open(newline="") as handle:
            reader = csv.DictReader(handle)
            for row in reader:
                rows.append(
                    (
                        float(row["B"]),
                        float(row["Pprime"]),
                        float(row["merit"]),
                        row.get("geometry", ""),
                    )
                )

        if not rows:
            print(f"  {path.name}: vazio")
            continue

        interior = [row for row in rows if 0.05 <= row[1] <= 0.95]
        if not interior:
            print(f"  {path.name}: sem pontos internos")
            continue

        bmin = min(row[0] for row in interior)
        bmax = max(row[0] for row in interior)
        pmin = min(row[1] for row in interior)
        pmax = max(row[1] for row in interior)
        geom = interior[0][3] or "n/a"
        print(
            f"  {path.name}: n={len(interior)} B=[{bmin:.3f},{bmax:.3f}] "
            f"P=[{pmin:.3f},{pmax:.3f}] geometry={geom}"
        )
    print()


def main() -> None:
    parser = argparse.ArgumentParser(description="Validacao numerica do solver do Goell.")
    parser.add_argument(
        "--table1",
        action="store_true",
        help="Compara os minimos encontrados com os valores publicados na Tabela I.",
    )
    parser.add_argument(
        "--figures",
        nargs="*",
        default=[],
        help="Resume os CSVs de uma ou mais figuras, por exemplo: fig16 fig17 fig18 fig19.",
    )
    parser.add_argument("--pscan", type=int, default=240, help="Numero de amostras em P para o modo table1.")
    parser.add_argument(
        "--stability-figures",
        nargs="*",
        default=[],
        help="Gera CSVs filtrados por estabilidade em N para uma ou mais figuras.",
    )
    parser.add_argument(
        "--n-values",
        default="5,7,9",
        help="Lista de N usada no filtro de estabilidade, por exemplo: 5,7,9",
    )
    parser.add_argument(
        "--min-support",
        type=int,
        default=2,
        help="Numero minimo de valores de N que devem sustentar um cluster estavel.",
    )
    parser.add_argument(
        "--p-tol",
        type=float,
        default=0.03,
        help="Tolerancia em P' usada para agrupar raizes de diferentes N.",
    )
    parser.add_argument(
        "--stability-pscan",
        type=int,
        default=240,
        help="Numero de amostras em P usado nas execucoes do filtro de estabilidade.",
    )
    parser.add_argument(
        "--det-search",
        default="minima",
        choices=("minima", "sign"),
        help="Modo de extracao usado quando metric=det: minima ou sign.",
    )
    args = parser.parse_args()

    if args.table1:
        compare_table1(pscan=args.pscan)

    for tag in args.figures:
        summarize_figure(tag)

    if args.stability_figures:
        n_values = parse_n_values(args.n_values)
        for tag in args.stability_figures:
            export_stable_figure(
                tag=tag,
                n_values=n_values,
                min_support=args.min_support,
                p_tol=args.p_tol,
                pscan=args.stability_pscan,
                metric="det",
                det_search=args.det_search,
                geometry="intersection",
                rescale=False,
            )


if __name__ == "__main__":
    main()
