#!/usr/bin/env python3
"""
field_map.py — Gera mapas de campo para um modo do guia Goell CHM.

Todos os cálculos físicos (Ez, Hz, Et, Ht) são feitos pelo binário C++
goell_q_solver com a flag --field-map.  Este script apenas:
  1. Chama o solver e lê o CSV resultante (stdlib csv + numpy).
  2. Faz o reshape para grade 2D.
  3. Plota 6 painéis (Ez, Hz, |Et|, |Ht|, setas E, setas H).
  4. Salva a figura em arquivo PNG.

Uso:
  python3 scripts/field_map.py \\
      --parity odd --phase phi0 --a_over_b 1 --nr 1.5 \\
      --N 5 --field-B 2.5 --field-P 0.65 \\
      --field-nx 80 --field-ny 80 --field-margin 1.4 \\
      --output figures/field_HE11.png

Todos os parâmetros após -- (exceto --output/-o) são passados ao solver.
"""

import argparse
import csv
import io
import subprocess
import sys
from pathlib import Path

import matplotlib.patches as mpatches
import matplotlib.pyplot as plt
import numpy as np

# ── Localização do binário ────────────────────────────────────────────────────
REPO_ROOT  = Path(__file__).resolve().parent.parent
SOLVER_BIN = REPO_ROOT / "build" / "goell_q_solver"


def parse_cli():
    """Separa --output/-o dos argumentos que vão ao solver."""
    args = sys.argv[1:]
    output = "field_map.png"
    solver_args = []
    i = 0
    while i < len(args):
        if args[i] in ("-o", "--output") and i + 1 < len(args):
            output = args[i + 1]
            i += 2
        elif args[i].startswith("--output="):
            output = args[i].split("=", 1)[1]
            i += 1
        elif args[i] in ("-h", "--help"):
            print(__doc__)
            sys.exit(0)
        else:
            solver_args.append(args[i])
            i += 1
    return output, solver_args


def call_solver(solver_args: list) -> dict:
    """Chama o binário C++ e retorna dict de arrays numpy por coluna."""
    if not SOLVER_BIN.exists():
        sys.exit(
            f"[field_map] Binário não encontrado: {SOLVER_BIN}\n"
            "Compile com:  ./run.sh build"
        )
    cmd = [str(SOLVER_BIN), "--field-map"] + solver_args
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"[field_map] Solver retornou erro:\n{result.stderr}")

    reader = csv.DictReader(io.StringIO(result.stdout))
    rows = list(reader)
    if not rows:
        sys.exit("[field_map] Solver retornou CSV vazio.")

    # Converte cada coluna para array numpy
    cols = {key: np.array([float(r[key]) for r in rows]) for key in rows[0]}
    return cols


def build_grids(cols: dict):
    """Converte colunas planas em grades 2D (reshape por valores únicos de x e y)."""
    xs = np.sort(np.unique(cols["x"]))
    ys = np.sort(np.unique(cols["y"]))
    nx, ny = len(xs), len(ys)

    # Ordena por y depois x para garantir o reshape correto
    order = np.lexsort((cols["x"], cols["y"]))
    def col2d(name):
        return cols[name][order].reshape(ny, nx)

    X, Y = np.meshgrid(xs, ys)
    return X, Y, {
        "Ez":     col2d("Ez"),
        "Hz":     col2d("Hz"),
        "Ex":     col2d("Ex"),
        "Ey":     col2d("Ey"),
        "Hx":     col2d("Hx"),
        "Hy":     col2d("Hy"),
        "Et":     np.hypot(col2d("Ex"), col2d("Ey")),
        "Ht":     np.hypot(col2d("Hx"), col2d("Hy")),
        "inside": col2d("inside").astype(bool),
    }


def add_core_rect(ax, a_over_b: float):
    """Adiciona contorno tracejado do núcleo retangular."""
    rect = mpatches.Rectangle(
        (-a_over_b / 2, -0.5), a_over_b, 1.0,
        linewidth=1.2, edgecolor="white", facecolor="none", linestyle="--",
    )
    ax.add_patch(rect)


def get_float_arg(solver_args: list, flag: str, default: float) -> float:
    for i, arg in enumerate(solver_args):
        if arg == flag and i + 1 < len(solver_args):
            return float(solver_args[i + 1])
    return default


def make_title(solver_args: list) -> str:
    keys = {"--parity": "par", "--phase": "φ", "--field-B": "B",
            "--field-P": "P'", "--nr": "nᵣ", "--a_over_b": "a/b", "--N": "N"}
    parts = []
    for flag, label in keys.items():
        for i, arg in enumerate(solver_args):
            if arg == flag and i + 1 < len(solver_args):
                parts.append(f"{label}={solver_args[i+1]}")
                break
    return "  ".join(parts)


def plot_field_map(cols: dict, solver_args: list, output: str):
    X, Y, G   = build_grids(cols)
    a_over_b  = get_float_arg(solver_args, "--a_over_b", 1.0)
    title     = make_title(solver_args)

    fig, axes = plt.subplots(2, 3, figsize=(14, 9))
    fig.suptitle(f"Mapa de Campo — {title}", fontsize=11)

    panels = [
        ("Ez",   G["Ez"],  "RdBu_r", r"$E_z$  (longitudinal)"),
        ("Hz",   G["Hz"],  "RdBu_r", r"$H_z$  (longitudinal)"),
        ("|Et|", G["Et"],  "inferno", r"$|E_t|$  (transversal elét.)"),
        ("|Ht|", G["Ht"],  "inferno", r"$|H_t|$  (transversal mag.)"),
        ("Et→",  None,     None,      r"$E_t$  (direção)"),
        ("Ht→",  None,     None,      r"$H_t$  (direção)"),
    ]

    # Subsampla as setas: ~18 por linha
    stride = max(1, X.shape[1] // 18)

    for ax, (key, data, cmap, label) in zip(axes.flat, panels):
        ax.set_aspect("equal")
        ax.set_title(label, fontsize=10)
        ax.set_xlabel("x / b")
        ax.set_ylabel("y / b")

        if data is not None:
            vmax = np.max(np.abs(data)) or 1.0
            vmin = -vmax if cmap == "RdBu_r" else 0.0
            im = ax.pcolormesh(X, Y, data, cmap=cmap, vmin=vmin, vmax=vmax,
                               shading="auto")
            fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
        else:
            # Quiver com direção normalizada (comprimento fixo)
            Fx = G["Ex"] if key == "Et→" else G["Hx"]
            Fy = G["Ey"] if key == "Et→" else G["Hy"]
            mag = np.hypot(Fx, Fy)
            mag[mag < 1e-9] = 1.0
            Xs = X[::stride, ::stride]
            Ys = Y[::stride, ::stride]
            Us = (Fx / mag)[::stride, ::stride]
            Vs = (Fy / mag)[::stride, ::stride]
            color = "steelblue" if key == "Et→" else "firebrick"
            ax.quiver(Xs, Ys, Us, Vs, pivot="mid", scale=25,
                      headwidth=3, color=color)
            # Plota |E_t| ou |H_t| como fundo para contexto
            bg = G["Et"] if key == "Et→" else G["Ht"]
            ax.pcolormesh(X, Y, bg, cmap="Greys", vmin=0,
                          vmax=np.max(bg) or 1.0, shading="auto", alpha=0.3)

        add_core_rect(ax, a_over_b)

    plt.tight_layout()
    Path(output).parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(output, dpi=150, bbox_inches="tight")
    print(f"[field_map] Figura salva em: {output}")


def main():
    output, solver_args = parse_cli()
    if not solver_args:
        print(__doc__)
        sys.exit(0)
    cols = call_solver(solver_args)
    plot_field_map(cols, solver_args, output)


if __name__ == "__main__":
    main()
