#!/usr/bin/env python3
"""
Rastreador simples de ramos para os CSVs gerados pelo goell_q_solver.

Ideia:
- cada CSV de entrada contem as raizes/minimos de uma classe do paper;
- em cada valor de B, ha zero, uma ou varias raizes P';
- este script tenta costurar essas raizes em ramos continuos ao longo de B.

Este utilitario e deliberadamente diagnostico. Ele nao "prova" que um ramo e
fisico; ele apenas ajuda a separar:
- cadeias longas e suaves ao longo de B;
- pontos isolados ou cadeias curtas, mais suspeitos de serem espurios.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class Point:
    B: float
    Pprime: float
    merit: float
    parity: str
    phase: str
    geometry: str


@dataclass
class Branch:
    branch_id: int
    points: list[Point] = field(default_factory=list)

    @property
    def last(self) -> Point:
        return self.points[-1]

    def slope_estimate(self) -> float:
        """Estima dP'/dB a partir dos 2 ultimos pontos.

        Usado para extrapolar a posicao esperada do ramo em B_next.
        Retorna 0.0 para ramos com menos de 2 pontos (sem historico de inclinacao).
        """
        if len(self.points) < 2:
            return 0.0
        p0, p1 = self.points[-2], self.points[-1]
        dB = p1.B - p0.B
        if abs(dB) < 1e-12:
            return 0.0
        return (p1.Pprime - p0.Pprime) / dB

    def predicted_Pprime(self, B_next: float) -> float:
        """Extrapola P' para B_next usando a inclinacao local do ramo.

        Para ramos com 1 ponto usa o valor atual sem correcao.
        Para ramos com 2+ pontos extrapola linearmente.
        """
        return self.last.Pprime + self.slope_estimate() * (B_next - self.last.B)

    def monotonic_up_fraction(self, tol: float) -> float:
        if len(self.points) <= 1:
            return 1.0
        good = 0
        total = 0
        for left, right in zip(self.points, self.points[1:]):
            total += 1
            if right.Pprime >= left.Pprime - tol:
                good += 1
        return good / total if total else 1.0


def load_points(path: Path) -> dict[float, list[Point]]:
    by_B: dict[float, list[Point]] = {}
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            point = Point(
                B=float(row["B"]),
                Pprime=float(row["Pprime"]),
                merit=float(row.get("merit", "nan")),
                parity=row.get("parity", ""),
                phase=row.get("phase", ""),
                geometry=row.get("geometry", ""),
            )
            by_B.setdefault(point.B, []).append(point)

    for B in by_B:
        by_B[B].sort(key=lambda point: point.Pprime)
    return by_B


def track_branches(
    by_B: dict[float, list[Point]],
    *,
    max_jump: float,
) -> list[Branch]:
    active: list[Branch] = []
    finished: list[Branch] = []
    next_id = 0

    for B in sorted(by_B):
        points = by_B[B]

        # Custo de associacao: distancia entre P' do ponto e o P' *predito* pelo ramo
        # via extrapolacao linear (slope dos 2 ultimos pontos).
        # Isso evita que ramos proximos troquem pontos quando se cruzam em B:
        # o ramo com inclinacao correta tem custo menor do que um ramo vizinho
        # que esta "indo em direcao errada".
        pairs: list[tuple[float, int, int]] = []
        for branch_index, branch in enumerate(active):
            predicted = branch.predicted_Pprime(B)
            for point_index, point in enumerate(points):
                cost = abs(point.Pprime - predicted)
                if cost <= max_jump:
                    pairs.append((cost, branch_index, point_index))
        pairs.sort()

        used_branches: set[int] = set()
        used_points: set[int] = set()

        # Pareamento guloso um-para-um. Para o numero pequeno de raizes por B,
        # isso ja ajuda bastante a evitar "roubo" de pontos entre ramos.
        for _cost, branch_index, point_index in pairs:
            if branch_index in used_branches or point_index in used_points:
                continue
            active[branch_index].points.append(points[point_index])
            used_branches.add(branch_index)
            used_points.add(point_index)

        still_active: list[Branch] = []
        for branch_index, branch in enumerate(active):
            if branch_index in used_branches:
                still_active.append(branch)
            else:
                finished.append(branch)
        active = still_active

        for point_index, point in enumerate(points):
            if point_index in used_points:
                continue
            active.append(Branch(branch_id=next_id, points=[point]))
            next_id += 1

    finished.extend(active)
    return finished


def filter_branches(
    branches: list[Branch],
    *,
    min_length: int,
    monotonic_tol: float,
    min_monotonic_fraction: float,
) -> list[Branch]:
    kept = []
    for branch in branches:
        if len(branch.points) < min_length:
            continue
        if branch.monotonic_up_fraction(monotonic_tol) < min_monotonic_fraction:
            continue
        kept.append(branch)
    kept.sort(key=lambda branch: (branch.points[0].B, branch.points[0].Pprime, -len(branch.points)))
    return kept


def write_tracked_csv(path: Path, branches: list[Branch]) -> None:
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "branch_id",
                "B",
                "Pprime",
                "merit",
                "branch_len",
                "parity",
                "phase",
                "geometry",
            ],
        )
        writer.writeheader()
        for new_branch_id, branch in enumerate(branches):
            branch_len = len(branch.points)
            for point in branch.points:
                writer.writerow(
                    {
                        "branch_id": new_branch_id,
                        "B": f"{point.B:.6f}",
                        "Pprime": f"{point.Pprime:.6f}",
                        "merit": f"{point.merit:.6f}",
                        "branch_len": branch_len,
                        "parity": point.parity,
                        "phase": point.phase,
                        "geometry": point.geometry,
                    }
                )


def print_summary(path: Path, branches: list[Branch], monotonic_tol: float) -> None:
    print(path.name)
    if not branches:
        print("  sem ramos apos o filtro")
        return

    for branch in branches:
        first = branch.points[0]
        last = branch.points[-1]
        frac = branch.monotonic_up_fraction(monotonic_tol)
        print(
            f"  branch={branch.branch_id} n={len(branch.points)} "
            f"B=[{first.B:.3f},{last.B:.3f}] "
            f"P=[{first.Pprime:.3f},{last.Pprime:.3f}] "
            f"mono={frac:.2f}"
        )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Costura ramos continuos a partir dos CSVs do Goell.")
    parser.add_argument("csvs", nargs="+", help="Um ou mais CSVs de entrada.")
    parser.add_argument("--out-dir", default="out", help="Diretorio onde os CSVs rastreados serao gravados.")
    parser.add_argument("--max-jump", type=float, default=0.12, help="Salto maximo permitido em P' entre dois B consecutivos.")
    parser.add_argument("--min-length", type=int, default=6, help="Numero minimo de pontos para manter um ramo.")
    parser.add_argument(
        "--monotonic-tol",
        type=float,
        default=0.02,
        help="Tolerancia usada na checagem de crescimento monotono em P'.",
    )
    parser.add_argument(
        "--min-monotonic-fraction",
        type=float,
        default=0.85,
        help="Fracao minima de passos monotonicamente crescentes para manter o ramo.",
    )
    parser.add_argument(
        "--suffix",
        default="_tracked",
        help="Sufixo acrescentado ao nome-base do CSV de saida.",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    out_dir = Path(args.out_dir)
    out_dir.mkdir(exist_ok=True)

    for csv_path in args.csvs:
        path = Path(csv_path)
        by_B = load_points(path)
        branches = track_branches(by_B, max_jump=args.max_jump)
        filtered = filter_branches(
            branches,
            min_length=args.min_length,
            monotonic_tol=args.monotonic_tol,
            min_monotonic_fraction=args.min_monotonic_fraction,
        )

        output_path = out_dir / f"{path.stem}{args.suffix}.csv"
        write_tracked_csv(output_path, filtered)
        print_summary(output_path, filtered, args.monotonic_tol)


if __name__ == "__main__":
    main()
