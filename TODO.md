# TODO

Pendencias tecnicas priorizadas depois do fechamento inicial da pasta `docs/` e da primeira rodada de codificacao do pipeline de reproducao.

## Status De Prontidao

- O repositorio esta pronto para estudo, manutencao, build reproduzivel, Tabela I, curvas das Figs. 16-22 e exportacao exploratoria de mapas de campo.
- O objetivo cientifico completo ainda nao foi atingido: faltam fechar a interpretacao modal final, validar os mapas de campo contra o artigo e resolver a nota de reescalonamento da p. 2144.

## Concluido Nesta Fase

- Revisado o bloco `H^{TA}` no solver C++.
- Separada a protecao numerica da avaliacao fisica dos argumentos `hr` e `pr`.
- Reexecutada a Tabela I com selecao autonoma e thresholds objetivos.
- Criado `run.sh` como interface publica de build, reproducao e validacao.
- Migrados os auxiliares Python para `scripts/`, com `scripts/solver_api.py` centralizando chamadas ao binario.
- Adicionado `scripts/validate_bessel.py` para conferir Bessel contra `scipy.special`.
- Estabilizado o pipeline das Figs. 16-19 com CSVs brutos, estaveis e rastreados.
- Adicionado scaffold das Figs. 20-22 para os dois modos principais.
- Restaurada a compatibilidade publica com `./run.sh` e `src/presets.sh` como shims para `scripts/`.
- Concluida a modularizacao do solver em `include/goell/`, `src/core/` e `src/main.cpp`.
- Reorganizados os scripts internos em `scripts/lib/`, `scripts/pipelines/`, `scripts/plotting/`, `scripts/tracking/` e `scripts/validation/`, com wrappers publicos preservados em `scripts/`.
- Unificado o build oficial no `Makefile`, com `scripts/run.sh` delegando a `make`.
- Adicionado `make check-cpp` como smoke test leve para CLI, layout, matching geométrico e dimensoes basicas da matriz `Q`.

## Validacao modal

- Fechar a correspondencia fisica entre `principal_phi0` / `principal_phi90` e os rotulos `E^y_11` / `E^x_11`.
- Comparar visualmente os PNGs finais das Figs. 16-22 contra o scan do artigo.
- Manter `even-rect-mode=square-split` apenas como diagnostico do setor `even`.

## Campos E Figs. 4-15

- Validar os mapas exportados por `scripts/field_map.py` contra as Figs. 4-15 do artigo, nao apenas como smoke test.
- Fixar presets canonicos por figura e modo para que a geracao de campos entre no fluxo reprodutivel oficial.
- Decidir se os casos de campos entram em `./run.sh` como subcomandos publicos ou se permanecem como utilitario dedicado em `scripts/field_map.py`.
- Registrar em documentacao permanente a interpretacao fisica dos paineis `Ez`, `Hz`, `|E_t|`, `|H_t|`, vetores `E_t` e vetores `H_t`.

## Concluido — Pipeline de Campos (F3.2)

- Implementado `compute_null_vector` (SVD da matriz Q) em `src/core/diagnostics.cpp`.
- Recuperados coeficientes modais `[aₙ|bₙ|cₙ|dₙ]ᵀ` a partir do vetor nulo.
- Avaliados todos os componentes `Ez, Hz, Er, Eθ, Hr, Hθ, Ex, Ey, Hx, Hy` em grade 2D
  (interior via Jₙ, exterior via Kₙ) em `src/core/field.cpp`.
- Normalização global (pico de |Ez|/|Hz| em toda a grade) para garantir saída limitada
  mesmo para modos perto do cutoff (Kₙ com p pequeno).
- Flag `--field-map` no binário C++ exporta CSV; `scripts/field_map.py` apenas plota.
- Smoke-test: `figures/field_mode0_B2.5.png` (HE₁₁ confinado) e
  `figures/field_mode1_B2.5.png` (modo cutoff) gerados com colorbars corretas.
- Observacao: esta etapa fecha a infraestrutura de campos, mas ainda nao a validacao final das Figs. 4-15.

## Arquitetura Atual

- Manter a equivalencia numerica entre `./run.sh validate`, `python3 scripts/validate_bessel.py` e o smoke test `make check-cpp`.
- Preservar a compatibilidade da CLI publica (`./run.sh`, flags do binario e esquema CSV) enquanto novas capacidades forem adicionadas.
- Atualizar `README.md`, `CONTRIBUTING.md`, `DEVELOPMENT.md`, `docs/README.md` e `docs/simbolos.md` sempre que a arquitetura modular mudar.

## Pendencias Tecnicas De Fundo

- Reavaliar a nota de reescalonamento da p. 2144 do paper.
- Registrar em documentacao permanente qualquer decisao nova sobre rotulagem modal.
