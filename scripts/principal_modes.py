#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
from pathlib import Path


@dataclass
class BranchSummary:
    branch_id: str
    rows: list[dict[str, str]]
    b_start: float
    b_end: float
    p_start: float
    p_end: float
    length: int


def load_branch_summaries(path: Path) -> list[BranchSummary]:
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        rows = list(reader)

    by_branch: dict[str, list[dict[str, str]]] = {}
    for row in rows:
        branch_id = row.get("branch_id", "").strip()
        if not branch_id:
            continue
        by_branch.setdefault(branch_id, []).append(row)

    summaries = []
    for branch_id, branch_rows in by_branch.items():
        branch_rows.sort(key=lambda row: float(row["B"]))
        summaries.append(
            BranchSummary(
                branch_id=branch_id,
                rows=branch_rows,
                b_start=float(branch_rows[0]["B"]),
                b_end=float(branch_rows[-1]["B"]),
                p_start=float(branch_rows[0]["Pprime"]),
                p_end=float(branch_rows[-1]["Pprime"]),
                length=len(branch_rows),
            )
        )

    return summaries


def select_principal_branch(
    summaries: list[BranchSummary],
    *,
    b_start_tol: float = 1e-9,
) -> BranchSummary:
    if not summaries:
        raise ValueError("nenhum ramo rastreado disponivel para selecao principal")

    min_b_start = min(summary.b_start for summary in summaries)
    pool = [
        summary
        for summary in summaries
        if summary.b_start <= min_b_start + b_start_tol
    ]
    pool.sort(
        key=lambda summary: (
            -summary.length,
            -summary.p_end,
            summary.p_start,
            int(summary.branch_id),
        )
    )
    return pool[0]


def export_principal_branch(
    input_path: Path,
    output_path: Path,
    *,
    curve_label: str = "",
    param_name: str = "",
    param_value: str = "",
    mode_alias: str = "",
) -> BranchSummary:
    summaries = load_branch_summaries(input_path)
    selected = select_principal_branch(summaries)

    fieldnames = list(selected.rows[0].keys())
    extra_fields = []
    for name, value in (
        ("curve_label", curve_label),
        ("param_name", param_name),
        ("param_value", param_value),
        ("mode_alias", mode_alias),
    ):
        if value and name not in fieldnames:
            extra_fields.append(name)
    fieldnames.extend(extra_fields)

    with output_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for row in selected.rows:
            out_row = dict(row)
            if curve_label:
                out_row["curve_label"] = curve_label
            if param_name:
                out_row["param_name"] = param_name
            if param_value:
                out_row["param_value"] = param_value
            if mode_alias:
                out_row["mode_alias"] = mode_alias
            writer.writerow(out_row)

    return selected


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Seleciona o ramo principal a partir de um CSV rastreado."
    )
    parser.add_argument("tracked_csvs", nargs="+", help="Um ou mais CSVs *_tracked.csv.")
    parser.add_argument("--out-dir", default="out", help="Diretorio de saida.")
    parser.add_argument("--param-name", default="", help="Nome do parametro varrido.")
    parser.add_argument("--param-value", default="", help="Valor do parametro varrido.")
    parser.add_argument("--mode-alias", default="", help="Alias para o modo principal.")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    out_dir = Path(args.out_dir)
    out_dir.mkdir(exist_ok=True)

    for csv_path in args.tracked_csvs:
        path = Path(csv_path)
        curve_label = path.stem.replace("_tracked", "")
        output_path = out_dir / f"{path.stem}_principal.csv"
        selected = export_principal_branch(
            path,
            output_path,
            curve_label=curve_label,
            param_name=args.param_name,
            param_value=args.param_value,
            mode_alias=args.mode_alias,
        )
        print(
            f"{output_path.name}: branch={selected.branch_id} "
            f"B=[{selected.b_start:.3f},{selected.b_end:.3f}] "
            f"P=[{selected.p_start:.3f},{selected.p_end:.3f}] "
            f"n={selected.length}"
        )


if __name__ == "__main__":
    main()
