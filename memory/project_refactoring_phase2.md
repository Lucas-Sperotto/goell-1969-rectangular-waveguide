---
name: Refatoração Fase 2 — concluída
description: Mudanças feitas na Fase 2 da refatoração dos scripts Python (2026-04-23)
type: project
---

Fase 2 concluída em 2026-04-23. Scripts movidos de src/ para scripts/ pelo usuário antes do início.

**Correção de paths (run.sh):**
- `REPO_ROOT` corrigido de `$SCRIPT_DIR` (= scripts/) para `$(dirname "$SCRIPT_DIR")` (= repo root)
- Todas as chamadas `src/*.py` → `scripts/*.py` (CPP_SRC permanece em `src/`)

**F2.2 — Novo scripts/solver_api.py:**
- `SOLVER_BIN = REPO_ROOT / "build" / "goell_q_solver"` usando `__file__`-relativo
- `ensure_solver_exists()` com mensagem de erro clara
- `call_solver(args)` que chama o binário e retorna CSV parseado
- Adotado por: reproduce_table1.py, validate_goell.py, analyze_table1_variation.py
- Eliminou: SOLVER_BIN duplicado, ensure_solver_exists duplicada, subprocess/io inline em 3 scripts

**F2.1 — reproduce_table1.py:**
- Removidos imports duplicados (io, subprocess, SOLVER_BIN, ensure_solver_exists)
- Passa a usar call_solver de solver_api
- Estrutura de funções já estava boa (run_solver_case → select_candidate → reproduce_table → write_* → main)

**F2.3 — track_roots.py — extrapolação linear:**
- Adicionados `Branch.slope_estimate()` e `Branch.predicted_Pprime(B_next)`
- `track_branches` agora usa `|point.Pprime - branch.predicted_Pprime(B)|` como custo (antes era `|point.Pprime - branch.last.Pprime|`)
- Melhora o rastreamento quando dois ramos se cruzam em B: o ramo com inclinação local correta vence o pareamento

**Resultado da validação pós-fase:**
- `bash scripts/run.sh validate` passou com mesmos números: RMSE=0.001226, MAE=0.000649, pior=0.005872
- `python3 scripts/validate_bessel.py` mantém 70/70 PASS

**Estrutura atual dos scripts:**
- `scripts/solver_api.py` — interface com o binário (novo)
- `scripts/reproduce_table1.py` — Tabela I (limpo)
- `scripts/validate_goell.py` — estabilidade + rastreamento (limpo)
- `scripts/analyze_table1_variation.py` — variantes (limpo)
- `scripts/track_roots.py` — rastreamento com extrapolação (melhorado)
- `scripts/validate_bessel.py` — testes de Bessel (da Fase 1)
- `scripts/run.sh` — orquestrador principal (corrigido)
