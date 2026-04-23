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
import math
from pathlib import Path
from statistics import fmean

from solver_api import call_solver, ensure_solver_exists
from track_roots import filter_branches, load_points, track_branches, write_tracked_csv


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
    even_rect_mode: str,
    rescale: bool,
) -> list[dict[str, str]]:
    args = [
        "--parity", parity, "--phase", phase, "--geometry", geometry,
        "--a_over_b", str(a_over_b), "--nr", str(nr), "--N", str(N),
        "--Bmin", str(B), "--Bmax", str(B), "--NB", "0",
        "--Pscan", str(pscan), "--metric", metric,
        "--det-search", det_search, "--even-rect-mode", even_rect_mode,
        "--dump-scan", str(B),
    ]
    if not rescale:
        args.append("--no-rescale")
    return call_solver(args)


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
    even_rect_mode: str,
    rescale: bool,
) -> list[dict[str, str]]:
    args = [
        "--parity", parity, "--phase", phase, "--geometry", geometry,
        "--a_over_b", str(a_over_b), "--nr", str(nr), "--N", str(N),
        "--Bmin", str(Bmin), "--Bmax", str(Bmax), "--NB", str(NB),
        "--Pscan", str(pscan), "--metric", metric,
        "--det-search", det_search, "--even-rect-mode", even_rect_mode,
        "--all-minima",
    ]
    if not rescale:
        args.append("--no-rescale")
    return call_solver(args)


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
    even_rect_mode: str,
    geometry: str,
    rescale: bool,
    tracked_max_jump: float,
    tracked_min_length: int,
    tracked_monotonic_tol: float,
    tracked_min_monotonic_fraction: float,
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
        f"support>={min_support}, p_tol={p_tol:.3f}, even-rect-mode={even_rect_mode}"
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
                even_rect_mode=even_rect_mode,
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
        tracked_path = track_stable_csv(
            out_path,
            max_jump=tracked_max_jump,
            min_length=tracked_min_length,
            monotonic_tol=tracked_monotonic_tol,
            min_monotonic_fraction=tracked_min_monotonic_fraction,
        )
        exported.append(tracked_path)

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
        print(f"  {tracked_path.name}: rastreado por continuidade em B")

    print()
    return exported


def track_stable_csv(
    path: Path,
    *,
    max_jump: float,
    min_length: int,
    monotonic_tol: float,
    min_monotonic_fraction: float,
) -> Path:
    by_B = load_points(path)
    branches = track_branches(by_B, max_jump=max_jump)
    kept = filter_branches(
        branches,
        min_length=min_length,
        monotonic_tol=monotonic_tol,
        min_monotonic_fraction=min_monotonic_fraction,
    )
    tracked_path = path.with_name(f"{path.stem}_tracked.csv")
    write_tracked_csv(tracked_path, kept)
    return tracked_path


def summarize_curve_path(path: Path) -> str:
    if not path.exists():
        return f"- `{path.name}`: arquivo ausente"

    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        rows = list(reader)

    if not rows:
        return f"- `{path.name}`: vazio"

    values = []
    branch_ids = set()
    for row in rows:
        try:
            B = float(row["B"])
            Pprime = float(row["Pprime"])
        except (KeyError, ValueError):
            continue
        if not math.isfinite(Pprime):
            continue
        values.append((B, Pprime))
        branch_id = row.get("branch_id", "").strip()
        if branch_id:
            branch_ids.add(branch_id)

    if not values:
        return f"- `{path.name}`: sem pontos finitos"

    bmin = min(B for B, _ in values)
    bmax = max(B for B, _ in values)
    pmin = min(P for _, P in values)
    pmax = max(P for _, P in values)
    return (
        f"- `{path.name}`: pontos={len(values)}, ramos={len(branch_ids)}, "
        f"B=[{bmin:.3f},{bmax:.3f}], P'=[{pmin:.3f},{pmax:.3f}]"
    )


def write_figure_report(
    *,
    tag: str,
    n_values: list[int],
    min_support: int,
    p_tol: float,
    det_search: str,
    even_rect_mode: str,
) -> Path:
    out_dir = Path("out")
    report_path = out_dir / f"{tag}_validation_summary.md"
    raw_paths = [out_dir / f"{tag}_{suffix}.csv" for _, _, suffix in CLASS_CASES]
    stable_paths = [out_dir / f"{tag}_stable_{suffix}.csv" for _, _, suffix in CLASS_CASES]
    tracked_paths = [out_dir / f"{tag}_stable_{suffix}_tracked.csv" for _, _, suffix in CLASS_CASES]

    lines = []
    lines.append(f"# Resumo De Validacao - {tag}")
    lines.append("")
    lines.append("Configuracao de estabilidade:")
    lines.append(f"- `det-search = {det_search}`")
    lines.append(f"- `even-rect-mode = {even_rect_mode}`")
    lines.append(f"- `n_values = {','.join(str(N) for N in n_values)}`")
    lines.append(f"- `min_support = {min_support}`")
    lines.append(f"- `p_tol = {p_tol:.3f}`")
    qualitative = qualitative_notes(tag)
    if qualitative:
        lines.append("")
        lines.append("## Leitura Qualitativa Esperada")
        lines.append("")
        for note in qualitative:
            lines.append(f"- {note}")
    lines.append("")
    lines.append("## CSVs Brutos")
    lines.append("")
    for path in raw_paths:
        lines.append(summarize_curve_path(path))
    lines.append("")
    lines.append("## CSVs Estaveis")
    lines.append("")
    for path in stable_paths:
        lines.append(summarize_curve_path(path))
    lines.append("")
    lines.append("## CSVs Rastreado")
    lines.append("")
    for path in tracked_paths:
        lines.append(summarize_curve_path(path))
    lines.append("")
    lines.append("Observacao:")
    lines.append(
        "- Os CSVs `stable` agrupam raizes por suporte em `N`; os CSVs `tracked` aplicam continuidade em `B` sobre esses agrupamentos."
    )
    report_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Relatorio: {report_path}")
    return report_path


def qualitative_notes(tag: str) -> list[str]:
    notes = {
        "fig16": [
            "Caso quadrado de baixo contraste: espera-se degenerescencia entre pares `E^y_mn` e `E^x_mn`.",
            "Os ramos `odd/phi0` e `odd/phi90` principais devem permanecer praticamente sobrepostos.",
        ],
        "fig17": [
            "Com `a/b = 2`, as degenerescencias geometricas devem se quebrar, exceto a dualidade `E^x_mn` / `E^y_mn` no limite de baixo contraste.",
            "A leitura modal final ainda depende da correspondencia fisica dos ramos, especialmente nos cruzamentos.",
        ],
        "fig18": [
            "Indice finito com razao de aspecto unitaria: espera-se levantamento parcial das degenerescencias.",
            "Modos cujas linhas de campo revertem ao atravessar a origem tendem a se separar mais claramente.",
        ],
        "fig19": [
            "Indice finito e `a/b = 2`: espera-se levantamento mais forte das degenerescencias.",
            "Este e o caso mais exigente para validar continuidade de ramos e rotulagem modal.",
        ],
    }
    return notes.get(tag, [])


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
                        even_rect_mode="paper",
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
        default="sign",
        choices=("minima", "sign"),
        help="Modo de extracao usado quando metric=det: minima ou sign.",
    )
    parser.add_argument(
        "--even-rect-mode",
        default="paper",
        choices=("paper", "square-split"),
        help="Regra usada no caso even com a/b != 1.",
    )
    parser.add_argument(
        "--report-figures",
        nargs="*",
        default=[],
        help="Escreve resumos em Markdown para uma ou mais figuras.",
    )
    parser.add_argument(
        "--tracked-max-jump",
        type=float,
        default=0.12,
        help="Salto maximo em P' para costurar os CSVs estaveis ao longo de B.",
    )
    parser.add_argument(
        "--tracked-min-length",
        type=int,
        default=6,
        help="Numero minimo de pontos para manter um ramo rastreado.",
    )
    parser.add_argument(
        "--tracked-monotonic-tol",
        type=float,
        default=0.02,
        help="Tolerancia de monotonicidade usada no rastreamento.",
    )
    parser.add_argument(
        "--tracked-min-monotonic-fraction",
        type=float,
        default=0.85,
        help="Fracao minima de passos monotonicamente crescentes no rastreamento.",
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
                even_rect_mode=args.even_rect_mode,
                geometry="intersection",
                rescale=False,
                tracked_max_jump=args.tracked_max_jump,
                tracked_min_length=args.tracked_min_length,
                tracked_monotonic_tol=args.tracked_monotonic_tol,
                tracked_min_monotonic_fraction=args.tracked_min_monotonic_fraction,
            )

    if args.report_figures:
        n_values = parse_n_values(args.n_values)
        for tag in args.report_figures:
            write_figure_report(
                tag=tag,
                n_values=n_values,
                min_support=args.min_support,
                p_tol=args.p_tol,
                det_search=args.det_search,
                even_rect_mode=args.even_rect_mode,
            )


if __name__ == "__main__":
    main()
