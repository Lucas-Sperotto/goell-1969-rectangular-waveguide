# Guia de desenvolvimento

Este documento reúne as convenções do repositório e o roteiro de evolução planejado. Destina-se a quem quer contribuir com código, scripts ou documentação.

## Estado atual do repositorio

- O solver principal agora entra por `src/main.cpp` e esta modularizado em `src/core/`, com interfaces em `include/goell/`.
- Os utilitarios de reproducao, validacao e plotagem continuam acessiveis em `scripts/`, com implementacao interna organizada em `scripts/lib`, `scripts/pipelines`, `scripts/plotting`, `scripts/tracking` e `scripts/validation`.
- `run.sh` na raiz e a interface publica; `scripts/run.sh` e a implementacao interna.
- `Makefile` e o build oficial do solver modular; `scripts/run.sh` delega a ele.
- `memory/` guarda apenas notas historicas da migracao, nao codigo oficial.
- A documentacao principal esta em `docs/`, com indice em `docs/README.md`.
- As notas permanentes de apoio ficam em `docs/referencias/`.
- `build/`, `out/` e `figures/` sao artefatos gerados localmente.

## Convencoes do projeto

### Nucleo numerico

- O nucleo numerico continua em **C++17** com dependencia principal de Eigen.
- Mudancas no solver devem citar, sempre que possivel, quais equacoes, figuras ou tabelas do paper estao sendo implementadas ou revisadas.
- O foco atual e o metodo de Goell para `det(Q) = 0`, nao modelos de outros artigos nem uma arquitetura de multiplos solvers inexistente neste snapshot.

### Entradas e saidas

- O fluxo principal gera CSVs em `out/` e figuras em `figures/`.
- Os graficos devem nascer dos artefatos numericos produzidos pelo solver e pelos scripts de apoio em `scripts/`.
- Quando um resultado for importante para a memoria tecnica do projeto, ele deve virar nota em `docs/` ou `docs/referencias/`, em vez de ficar preso apenas a um artefato temporario.

### Documentacao e rastreabilidade

- Ambiguidades de OCR no artigo original devem ser marcadas com `TODO`.
- Divergencias entre reproducao e scan devem ser classificadas como cientificas ou editoriais.
- Se um arquivo em `docs/` for renomeado, movido ou promovido de categoria, atualizar tambem `docs/README.md`, o `README.md` da raiz e os links internos relevantes.

### Verificacao minima

- Recompilar o solver quando houver mudanca em `src/main.cpp`, `src/core/` ou `include/goell/`.
- Executar `python3 -m py_compile` nos scripts Python alterados.
- Se a mudanca for numerica, rerodar ao menos o fluxo mais proximo do trecho afetado, preferencialmente via `./run.sh`, `scripts/reproduce_table1.py` ou `scripts/validate_goell.py`.

## Mapa artigo -> codigo

| Parte do artigo | Arquivos principais |
| --- | --- |
| Secao II - formulacao teorica | `src/core/matrix.cpp`, `src/core/boundary.cpp`, `src/core/layout.cpp`, `docs/02_derivacao_das_equacoes.md`, `docs/02.1_efeitos_da_simetria.md` a `docs/02.7_metodo_de_computacao.md` |
| Secao III - resultados | `run.sh`, `scripts/plot_compare.py`, `scripts/validate_goell.py`, `docs/03_resultados_do_calculo.md`, `docs/03.1_precisao.md`, `docs/03.2_configuracoes_modais.md`, `docs/03.3_curvas_de_propagacao.md` |
| Tabela I | `scripts/reproduce_table1.py`, `scripts/analyze_table1_variation.py`, `docs/referencias/04_notas_sobre_a_tabela_1.md` |
| Figs. 4-15 / mapas de campo | `src/core/field.cpp`, `include/goell/field.hpp`, `src/core/diagnostics.cpp`, `scripts/field_map.py`, `docs/02.5_diferencas_entre_as_funcoes_de_campo_eletrico_e_magnetico.md` |
| Modos principais e Figs. 20-22 | `scripts/principal_modes.py`, `scripts/sweep_principal_modes.py`, `run.sh`, `docs/03.3_curvas_de_propagacao.md` |
| Arquitetura atual do solver | `include/goell/*.hpp`, `src/main.cpp`, `src/core/*.cpp`, `memory/project_refactoring_phase2.md` |
| Conferencia de notacao e OCR | `docs/referencias/03_checklist_de_conferencia.md` |

## Comandos uteis

```bash
./run.sh build
make check-cpp
./run.sh table1
./run.sh fig16
./run.sh fig17
./run.sh fig20
./run.sh fig22
./run.sh all-curves
SWEEP_REUSE_EXISTING=1 ./run.sh fig22
./run.sh validate
python3 scripts/analyze_table1_variation.py
python3 scripts/validate_goell.py --stability-figures fig16 fig17 --det-search sign
python3 scripts/validate_bessel.py
python3 scripts/field_map.py --parity odd --phase phi0 --a_over_b 1 --nr 1.5 --N 5 --field-B 2.5 --field-P 0.65 --field-nx 80 --field-ny 80 --field-margin 1.4 --output figures/field_HE11.png
```

## Roteiro de evolucao

A sequencia recomendada de proximas contribuicoes, em ordem de prioridade, continua sendo a que esta consolidada em [TODO.md](TODO.md):

1. Consolidar a leitura fisica e a identificacao modal final das curvas exportadas nas Figs. 16-19.
2. Refinar a rotulagem fisica dos modos principais nas Figs. 20-22.
3. Validar os mapas de campo exportados contra as Figs. 4-15 e fechar presets canonicos para esses casos.
4. Resolver a nota de reescalonamento da p. 2144 sem perder rastreabilidade com o paper.

## Avancos recentes

- `run.sh` virou a interface publica oficial do repositorio.
- O modo canonico de reproducao passou a ser busca por mudanca de sinal de `det(Q)`.
- O bloco `H^{TA}` e a separacao entre protecao numerica e avaliacao fisica foram revisados no solver.
- A regra do caso `even` com `a/b != 1` agora esta travada explicitamente como `paper`, com `square-split` mantido apenas para diagnostico.
- A Tabela I agora roda em modo autonomo e com criterio numerico objetivo de aceite.
- As Figs. 16-19 ja possuem pipeline de CSV bruto, CSV estavel, CSV rastreado e figura final.
- As Figs. 20-22 ja possuem sweep dos modos principais, com retomada opcional via `SWEEP_REUSE_EXISTING=1`.
- O repositório agora tambem seleciona e exporta os dois modos principais em sweeps parametricos, preparando a reproducao das Figs. 20-22.
- O solver agora tambem exporta grades de campo 2D por `--field-map`, com plotagem desacoplada em `scripts/field_map.py`.

## O que não fazer cedo demais

- Refatorar a arquitetura do projeto para diretórios que ainda nao existem so por imitacao de outros repositorios.
- Introduzir uma camada nova de build ou de automacao sem necessidade concreta.
- Perseguir fac-simile visual completo antes de fechar ambiguidades cientificas e modais.
- Documentar o repositório com referencias a comandos, scripts ou estruturas inexistentes no snapshot atual.
