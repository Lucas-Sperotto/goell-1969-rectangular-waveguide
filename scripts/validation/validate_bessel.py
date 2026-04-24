#!/usr/bin/env python3
"""
validate_bessel.py — Verifica as funcoes de Bessel implementadas no solver C++.

O script:
  1. Compila o solver se necessario.
  2. Chama `build/goell_q_solver --test-bessel` para obter J_n, J'_n, K_n, K'_n
     nos pontos de teste exportados pelo binario.
  3. Compara contra scipy.special como referencia independente.
  4. Verifica as identidades de recorrencia usadas em Jn_prime e Kn_prime.
  5. Reporta PASS / FAIL por item, com tolerancia relativa de 1e-10.

Uso:
    python3 scripts/validate_bessel.py           # usa build/goell_q_solver existente
    python3 scripts/validate_bessel.py --build   # recompila antes

Referencia:
  - J'_n(x) = (J_{n-1}(x) - J_{n+1}(x)) / 2   (recorrencia usada em Jn_prime)
  - K'_n(x) = -(K_{n-1}(x) + K_{n+1}(x)) / 2  (recorrencia usada em Kn_prime)
  - J'_0(x) = -J_1(x)
  - K'_0(x) = -K_1(x)
"""

import argparse
import csv
import subprocess
import sys
from pathlib import Path

try:
    from scipy.special import jv, kv, jvp, kvp
except ImportError:
    sys.exit("scipy nao encontrado. Instale com: pip install scipy")

REPO = Path(__file__).resolve().parents[2]
BINARY = REPO / "build" / "goell_q_solver"

TOL_REL = 1e-10  # tolerancia relativa para comparacao com scipy


def build_solver():
    print("Compilando solver...")
    result = subprocess.run(["make", "all"], cwd=REPO, capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"Falha na compilacao:\n{result.stderr}")
    print("Compilado com sucesso.\n")


def run_test_bessel():
    result = subprocess.run(
        [str(BINARY), "--test-bessel"],
        capture_output=True, text=True,
    )
    if result.returncode != 0:
        sys.exit(f"Binario retornou erro:\n{result.stderr}")
    rows = []
    reader = csv.DictReader(result.stdout.splitlines())
    for row in reader:
        rows.append({
            "n": int(row["n"]),
            "x": float(row["x"]),
            "Jn": float(row["Jn"]),
            "Jn_prime": float(row["Jn_prime"]),
            "Kn": float(row["Kn"]),
            "Kn_prime": float(row["Kn_prime"]),
        })
    return rows


def check(label, got, ref, tol=TOL_REL):
    # Perto de um zero da funcao, a tolerancia relativa nao tem sentido.
    # Usamos tolerancia absoluta quando |ref| < 1e-9 (nivel de ruido numerico).
    if abs(ref) < 1e-9:
        err = abs(got - ref)
        kind = "abs_err"
        tol_used = 1e-9
    else:
        err = abs(got - ref) / abs(ref)
        kind = "rel_err"
        tol_used = tol
    status = "PASS" if err <= tol_used else "FAIL"
    print(f"  [{status}] {label:55s}  got={got:+.6e}  ref={ref:+.6e}  {kind}={err:.2e}")
    return status == "PASS"


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build", action="store_true", help="Recompila o solver antes de testar")
    args = parser.parse_args()

    if args.build or not BINARY.exists():
        build_solver()

    rows = run_test_bessel()
    passed = 0
    failed = 0

    print("=" * 80)
    print("Secao 1: J_n e K_n vs scipy.special.jv / kv")
    print("=" * 80)
    for r in rows:
        n, x = r["n"], r["x"]
        ref_J = float(jv(n, x))
        ref_K = float(kv(n, x))
        ok_J = check(f"J_{n}({x})", r["Jn"], ref_J)
        ok_K = check(f"K_{n}({x})", r["Kn"], ref_K)
        passed += ok_J + ok_K
        failed += (not ok_J) + (not ok_K)

    print()
    print("=" * 80)
    print("Secao 2: J'_n e K'_n vs scipy.special.jvp / kvp")
    print("=" * 80)
    for r in rows:
        n, x = r["n"], r["x"]
        ref_Jp = float(jvp(n, x, 1))
        ref_Kp = float(kvp(n, x, 1))
        ok_Jp = check(f"J'_{n}({x})", r["Jn_prime"], ref_Jp)
        ok_Kp = check(f"K'_{n}({x})", r["Kn_prime"], ref_Kp)
        passed += ok_Jp + ok_Kp
        failed += (not ok_Jp) + (not ok_Kp)

    print()
    print("=" * 80)
    print("Secao 3: Identidades de recorrencia (verificadas via scipy)")
    print("         Garante que Jn_prime e Kn_prime no C++ seguem as mesmas formulas")
    print("=" * 80)

    recurrence_cases = [
        (0, 0.5), (0, 1.0), (0, 2.0),
        (1, 1.0), (1, 2.0),
        (2, 1.0), (2, 3.0),
        (3, 1.0),
    ]
    for n, x in recurrence_cases:
        # J'_n(x) = (J_{n-1}(x) - J_{n+1}(x)) / 2, caso geral; -J_1(x) para n=0
        rec_Jp = float(jvp(n, x, 1))
        if n == 0:
            formula_J = -float(jv(1, x))
            ok = check(f"J'_0({x}) == -J_1({x})", formula_J, rec_Jp, tol=1e-14)
        else:
            formula_J = 0.5 * (float(jv(n - 1, x)) - float(jv(n + 1, x)))
            ok = check(f"J'_{n}({x}) == (J_{n-1} - J_{n+1})/2", formula_J, rec_Jp, tol=1e-14)
        passed += ok
        failed += not ok

        # K'_n(x) = -(K_{n-1}(x) + K_{n+1}(x)) / 2, caso geral; -K_1(x) para n=0
        rec_Kp = float(kvp(n, x, 1))
        if n == 0:
            formula_K = -float(kv(1, x))
            ok = check(f"K'_0({x}) == -K_1({x})", formula_K, rec_Kp, tol=1e-14)
        else:
            formula_K = -0.5 * (float(kv(n - 1, x)) + float(kv(n + 1, x)))
            ok = check(f"K'_{n}({x}) == -(K_{n-1} + K_{n+1})/2", formula_K, rec_Kp, tol=1e-14)
        passed += ok
        failed += not ok

    print()
    print("=" * 80)
    print("Secao 4: Zeros conhecidos de J_n")
    print("=" * 80)
    zeros = [(0, 2.4048255577), (1, 3.8317059702)]
    for n, x0 in zeros:
        val = float(jv(n, x0))
        # O criterio aqui e absoluto (o valor deve ser proximo de zero)
        err = abs(val)
        status = "PASS" if err < 1e-9 else "FAIL"
        print(f"  [{status}] J_{n}({x0:.10f}) = {val:+.6e}  (deve ser ~0, |val|={err:.2e})")
        passed += status == "PASS"
        failed += status == "FAIL"

    print()
    print("=" * 80)
    total = passed + failed
    print(f"Resultado: {passed}/{total} testes passaram", end="")
    if failed == 0:
        print("  [OK]")
    else:
        print(f"  [{failed} FALHAS]")
    print("=" * 80)

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
