# Plano De Codificacao Para Reproduzir Goell 1969

_Navegacao: [README geral](../../README.md) | [Indice de docs](../README.md) | [Referencia anterior](04_notas_sobre_a_tabela_1.md)_

Este arquivo consolida o plano de codificacao usado para transformar a traducao e a auditoria tecnica do artigo de Goell em um pipeline executavel.

## Arquitetura

- `sh`: orquestracao unica via `./run.sh`.
- `C++17`: solver modular em `include/goell/`, `src/core/` e `src/main.cpp`, cobrindo CLI, layout, geometria, matriz `Q`, diagnosticos, campos, busca de raizes e exportacao.
- `Python`: wrappers publicos em `scripts/` e implementacao interna em `scripts/lib/`, `scripts/pipelines/`, `scripts/plotting/`, `scripts/tracking/` e `scripts/validation/`.

O principio adotado e reproducao numerica incremental: primeiro fechar o nucleo validavel do artigo, depois expandir para curvas adicionais e, por fim, campos.

## Nucleo Fechado Nesta Fase

- `./run.sh` e a interface publica oficial.
- `scripts/run.sh` contem a orquestracao interna; `./run.sh` e `src/presets.sh` sao shims de compatibilidade.
- `Makefile` e o build oficial do solver modular; `scripts/run.sh` delega a ele.
- `det-search=sign` e o modo canonico de reproducao.
- `geometry=intersection` e a geometria padrao.
- A Tabela I usa selecao autonoma do primeiro ramo fisico.
- As Figs. 16-19 usam CSVs brutos, CSVs estaveis, CSVs rastreados e PNG final.
- A regra `even` para `a/b != 1` fica travada como `paper`; `square-split` e diagnostico.
- As Figs. 20-22 usam o pipeline dos dois modos principais, ainda com rotulos conservadores `principal_phi0` e `principal_phi90`.
- O solver ja exporta grades 2D de campo com `--field-map`, e `scripts/field_map.py` plota os paineis exploratorios correspondentes.

## Defaults Canonicos

- Tabela I: `geometry=intersection`, `metric=det`, `det-search=sign`, `n_r=1.01`, `B=2`.
- Figs. 16-17: `n_r=1.0001`, como aproximacao fixa de `Delta n_r -> 0`.
- Figs. 18-19: `n_r=1.5`.
- Estabilidade modal: `N={5,7,9}`, `p_tol=0.03`, `min_support=2`.

## Criterios De Aceite

- `./run.sh build` compila a partir de estado limpo.
- `make check-cpp` passa como smoke test do nucleo modular.
- `./run.sh table1` gera CSV, Markdown e PNG.
- Tabela I mantem `MAE <= 0.01` e pior erro absoluto `<= 0.02`.
- `./run.sh fig16` a `./run.sh fig19` geram CSVs brutos, CSVs estaveis, CSVs rastreados, PNG debug, PNG final e resumo Markdown.
- `./run.sh fig20`, `fig21` e `fig22` geram CSV, Markdown e PNG dos modos principais.
- `./run.sh validate` consolida a Tabela I e a exportacao estavel de `fig16` e `fig17`, falhando se os thresholds numericos forem violados.
- `python3 scripts/validate_bessel.py` continua validando as funcoes de Bessel com 70/70 PASS.
- `python3 scripts/field_map.py --parity odd --phase phi0 --a_over_b 1 --nr 1.5 --N 5 --field-B 2.5 --field-P 0.65 --field-nx 80 --field-ny 80 --field-margin 1.4 --output figures/field_HE11.png` gera um smoke test dos campos.

## Etapa Seguinte

A proxima etapa nao e mais construir a infraestrutura de campos, e sim fechar a reproducao cientifica dessa parte do artigo:

- validar os mapas exportados contra as Figs. 4-15 do scan;
- fixar presets canonicos e nomenclatura modal para esses casos;
- decidir se a geracao de campos passa a integrar `./run.sh` como fluxo publico adicional;
- registrar qualquer ajuste de interpretacao em documentacao permanente.

_Fim da nota. Voltar ao [Indice de docs](../README.md)._
