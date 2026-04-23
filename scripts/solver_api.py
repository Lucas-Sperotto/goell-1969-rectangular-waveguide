"""
solver_api.py — Interface centralizada com o binário goell_q_solver.

Responsabilidades:
  - Localizar o binário em relacao a este arquivo (__file__-relativo),
    independente do diretorio de trabalho atual.
  - Verificar se o binario existe antes de chamar.
  - Chamar o binario, capturar stdout e parsear como CSV.

Uso:
    from solver_api import call_solver, ensure_solver_exists, SOLVER_BIN

    rows = call_solver([
        "--parity", "odd", "--phase", "phi0",
        "--a_over_b", "1", "--nr", "1.01",
        "--N", "5", "--Bmin", "2", "--Bmax", "2", "--NB", "0",
        "--Pscan", "240", "--metric", "det", "--det-search", "sign",
        "--even-rect-mode", "paper", "--all-minima", "--no-rescale",
    ])
    # rows e uma lista de dicts com as colunas do CSV do solver.
"""

from __future__ import annotations

import csv
import io
import subprocess
from pathlib import Path

# O binario fica em <repo_root>/build/. Este arquivo fica em <repo_root>/scripts/.
REPO_ROOT: Path = Path(__file__).resolve().parent.parent
SOLVER_BIN: Path = REPO_ROOT / "build" / "goell_q_solver"


def ensure_solver_exists() -> None:
    """Aborta com mensagem clara se o binario nao foi compilado."""
    if SOLVER_BIN.exists():
        return
    raise SystemExit(
        f"{SOLVER_BIN} nao encontrado.\n"
        "Compile com:  ./run.sh build\n"
        "Ou diretamente: g++ -O3 -std=c++17 src/goell_q_solver.cpp "
        "-I /usr/include/eigen3 -o build/goell_q_solver"
    )


def call_solver(args: list[str]) -> list[dict[str, str]]:
    """Chama o solver com os argumentos dados e retorna as linhas do CSV."""
    ensure_solver_exists()
    cmd = [str(SOLVER_BIN)] + args
    out = subprocess.check_output(cmd, text=True)
    return list(csv.DictReader(io.StringIO(out)))
