# TODO

Pendencias tecnicas priorizadas depois do fechamento inicial da pasta `docs/` e da primeira rodada de codificacao do pipeline de reproducao.

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

## Proxima Fase: Campos

- Implementar calculo do vetor nulo da matriz `Q`.
- Recuperar coeficientes modais a partir do vetor nulo.
- Amostrar campos em grid para as Figs. 4-15.
- Exportar CSVs de intensidade e componentes de campo.
- Gerar mapas de intensidade em Python.
- Planejar, depois dos mapas de intensidade, linhas de campo.

## Arquitetura Atual

- Manter a equivalencia numerica entre `./run.sh validate`, `python3 scripts/validate_bessel.py` e o smoke test `make check-cpp`.
- Preservar a compatibilidade da CLI publica (`./run.sh`, flags do binario e esquema CSV) enquanto novas capacidades forem adicionadas.
- Atualizar `README.md`, `CONTRIBUTING.md`, `DEVELOPMENT.md`, `docs/README.md` e `docs/simbolos.md` sempre que a arquitetura modular mudar.

## Pendencias Tecnicas De Fundo

- Reavaliar a nota de reescalonamento da p. 2144 do paper.
- Registrar em documentacao permanente qualquer decisao nova sobre rotulagem modal.
