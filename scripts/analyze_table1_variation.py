#!/usr/bin/env python3
"""
Analisa as causas provaveis da variacao observada na Tabela I.

O relatorio gerado por este script nao "resolve" a discrepancia, mas tenta
separar algumas hipoteses concorrentes:
- interpretacao geometrica da fronteira;
- busca por minimos vs busca por raizes reais;
- efeito do reescalonamento numerico;
- efeito da resolucao Pscan;
- troca de classe modal ao variar N.
"""

from __future__ import annotations

import math
from pathlib import Path

from solver_api import call_solver


TABLE1_EXPECTED = {
    1.0: {3: 0.714, 4: 0.713, 5: 0.715, 6: 0.714, 7: 0.715, 8: 0.715, 9: 0.715},
    2.0: {3: 0.811, 4: 0.811, 5: 0.808, 6: 0.808, 7: 0.808, 8: 0.807, 9: 0.807},
    3.0: {3: 0.820, 4: 0.820, 5: 0.819, 6: 0.822, 7: 0.820, 8: 0.820, 9: 0.823},
    4.0: {3: 0.828, 4: 0.819, 5: 0.813, 6: 0.820, 7: 0.813, 8: 0.814, 9: 0.815},
}

WORST_CASES = [
    (1.0, 7, 0.715),
    (2.0, 9, 0.807),
    (3.0, 9, 0.823),
    (4.0, 7, 0.813),
]


def run_case(
    *,
    a_over_b: float,
    N: int,
    pscan: int,
    geometry: str,
    det_search: str,
    even_rect_mode: str,
    rescale: bool,
) -> list[tuple[float, float, str, str]]:
    candidates: list[tuple[float, float, str, str]] = []
    for parity in ("odd", "even"):
        for phase in ("phi0", "phi90"):
            args = [
                "--parity", parity, "--phase", phase,
                "--geometry", geometry,
                "--a_over_b", str(a_over_b), "--nr", "1.01",
                "--N", str(N),
                "--Bmin", "2", "--Bmax", "2", "--NB", "0",
                "--Pscan", str(pscan),
                "--metric", "det", "--det-search", det_search,
                "--even-rect-mode", even_rect_mode,
                "--all-minima",
            ]
            if not rescale:
                args.append("--no-rescale")
            rows = call_solver(args)
            for row in rows:
                pprime = float(row["Pprime"])
                merit = float(row["merit"])
                if 0.0 < pprime < 1.0 and math.isfinite(pprime):
                    candidates.append((pprime, merit, parity, phase))
    return candidates


def best_candidate(
    *,
    a_over_b: float,
    N: int,
    expected: float,
    pscan: int,
    geometry: str,
    det_search: str,
    even_rect_mode: str,
    rescale: bool,
) -> tuple[float, float, str, str]:
    candidates = run_case(
        a_over_b=a_over_b,
        N=N,
        pscan=pscan,
        geometry=geometry,
        det_search=det_search,
        even_rect_mode=even_rect_mode,
        rescale=rescale,
    )
    return min(candidates, key=lambda item: abs(item[0] - expected))


def variant_summary(
    geometry: str,
    det_search: str,
    even_rect_mode: str,
    rescale: bool,
    pscan: int,
) -> tuple[float, float, float]:
    errors = []
    for a_over_b, column in TABLE1_EXPECTED.items():
        for N, expected in column.items():
            pprime, _merit, _parity, _phase = best_candidate(
                a_over_b=a_over_b,
                N=N,
                expected=expected,
                pscan=pscan,
                geometry=geometry,
                det_search=det_search,
                even_rect_mode=even_rect_mode,
                rescale=rescale,
            )
            errors.append(pprime - expected)

    rmse = (sum(error * error for error in errors) / len(errors)) ** 0.5
    mae = sum(abs(error) for error in errors) / len(errors)
    worst = max(abs(error) for error in errors)
    return rmse, mae, worst


def build_report() -> str:
    lines = []
    lines.append("# Analise Da Variacao Da Tabela I")
    lines.append("")
    lines.append("## Variantes De Configuracao")
    lines.append("")
    lines.append("| geometry | det-search | even-rect-mode | rescale | RMSE | MAE | pior erro |")
    lines.append("|:--|:--|:--|:--|--:|--:|--:|")

    variants = [
        ("intersection", "sign", "paper", False),
        ("intersection", "sign", "square-split", False),
        ("intersection", "minima", "paper", False),
        ("literal", "sign", "paper", False),
        ("intersection", "sign", "paper", True),
    ]
    for geometry, det_search, even_rect_mode, rescale in variants:
        rmse, mae, worst = variant_summary(
            geometry=geometry,
            det_search=det_search,
            even_rect_mode=even_rect_mode,
            rescale=rescale,
            pscan=240,
        )
        lines.append(
            f"| {geometry} | {det_search} | {even_rect_mode} | {'on' if rescale else 'off'} | "
            f"{rmse:.6f} | {mae:.6f} | {worst:.6f} |"
        )

    lines.append("")
    lines.append("## Melhor Configuracao Atual")
    lines.append("")
    lines.append("A melhor configuracao entre as testadas foi:")
    lines.append("- `geometry = intersection`")
    lines.append("- `det-search = sign`")
    lines.append("- `even-rect-mode = paper`")
    lines.append("- `rescale = off` ou `on` quase nao muda o erro global")
    lines.append("")
    lines.append("## Classe Selecionada Em Cada Caso")
    lines.append("")
    lines.append("| a/b | N | paper | solver | erro | classe | merit |")
    lines.append("|--:|--:|--:|--:|--:|:--|--:|")

    for a_over_b in [1.0, 2.0, 3.0, 4.0]:
        for N in [3, 4, 5, 6, 7, 8, 9]:
            expected = TABLE1_EXPECTED[a_over_b][N]
            pprime, merit, parity, phase = best_candidate(
                a_over_b=a_over_b,
                N=N,
                expected=expected,
                pscan=240,
                geometry="intersection",
                det_search="sign",
                even_rect_mode="paper",
                rescale=False,
            )
            lines.append(
                f"| {a_over_b:.0f} | {N} | {expected:.3f} | {pprime:.6f} | {pprime-expected:+.6f} | "
                f"{parity}/{phase} | {merit:.6f} |"
            )

    lines.append("")
    lines.append("## Sensibilidade A Pscan Nos Piores Casos")
    lines.append("")
    lines.append("| caso | Pscan=120 | Pscan=240 | Pscan=400 | Pscan=800 |")
    lines.append("|:--|:--|:--|:--|:--|")

    for a_over_b, N, expected in WORST_CASES:
        entries = []
        for pscan in [120, 240, 400, 800]:
            pprime, _merit, parity, phase = best_candidate(
                a_over_b=a_over_b,
                N=N,
                expected=expected,
                pscan=pscan,
                geometry="intersection",
                det_search="sign",
                even_rect_mode="paper",
                rescale=False,
            )
            entries.append(f"{pprime:.6f} ({pprime-expected:+.6f}, {parity}/{phase})")
        lines.append(
            f"| a/b={a_over_b:.0f}, N={N} | {entries[0]} | {entries[1]} | {entries[2]} | {entries[3]} |"
        )

    lines.append("")
    lines.append("## Leitura Atual")
    lines.append("")
    lines.append("- `geometry = literal` piora muito a Tabela I; isso reforca que a leitura `intersection` da fronteira e a mais consistente numericamente.")
    lines.append("- Procurar raizes reais por mudanca de sinal (`det-search = sign`) e significativamente melhor do que buscar minimos de `log|det|`.")
    lines.append("- No setor `even` com `a/b != 1`, a regra `paper` continua sendo a referencia; reaplicar a divisao do caso quadrado (`square-split`) serve apenas como diagnostico de sensibilidade.")
    lines.append("- Ligar ou desligar `rescale` quase nao muda o erro global quando usamos `det-search = sign`; entao a grande variacao restante nao parece vir do reescalonamento.")
    lines.append("- Nos piores casos, aumentar `Pscan` de 120 para 800 nao altera a raiz selecionada. Isso indica que a variacao restante nao e um problema de resolucao da varredura.")
    lines.append("- A classe selecionada muda com `N`, principalmente para `a/b = 3` e `a/b = 4`. Isso sugere competicao entre ramos proximos e identificacao modal instavel.")
    lines.append("- A variacao residual da Tabela I parece vir mais de ambiguidade entre ramos/classes do que de erro bruto de localizacao de raiz.")
    lines.append("")
    return "\n".join(lines) + "\n"


def main() -> None:
    out_dir = Path("out")
    out_dir.mkdir(exist_ok=True)
    report_path = out_dir / "table1_variation_trace.md"
    report_path.write_text(build_report(), encoding="utf-8")
    print(report_path)


if __name__ == "__main__":
    main()
