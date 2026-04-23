# Goell 1969 - Reproducao, Estudo E Implementacao

Este repositorio existe para estudar e reproduzir numericamente o artigo:

J. E. Goell, *A Circular-Harmonic Computer Analysis of Rectangular Dielectric Waveguides*, *Bell System Technical Journal*, setembro de 1969.

Ele foi organizado para cumprir tres papeis ao mesmo tempo:

- servir como guia de estudo em portugues;
- concentrar uma implementacao executavel do metodo;
- registrar as tentativas de validacao contra tabelas e figuras do artigo.

## Visao Geral

O metodo de Goell parte de uma ideia elegante: em vez de descrever o guia retangular por solucoes cartesianas em varias regioes, ele expande os campos em harmonicos circulares e faz o casamento dos campos tangenciais ao longo da fronteira do nucleo.

Em termos de fluxo de trabalho, isso significa:

1. expandir `E_z` e `H_z` em series com funcoes de Bessel no interior e funcoes de Bessel modificadas no exterior;
2. reconstruir os campos transversais a partir das equacoes de Maxwell;
3. escolher classes modais usando simetria;
4. impor o casamento em pontos da fronteira;
5. montar a matriz global `Q`;
6. procurar os parametros para os quais `det(Q) = 0`.

Este repositorio foi montado justamente para tornar esse caminho legivel, verificavel e reproduzivel.

## Estrutura Do Repositorio

### Codigo

- [run.sh](run.sh): interface publica oficial para build, Tabela I, Figs. 16-19, validacao e rodada completa.
- [src/goell_q_solver.cpp](src/goell_q_solver.cpp): solver principal, com montagem da matriz `Q`, busca de raizes e exportacao das curvas.
- [src/presets.sh](src/presets.sh): shim de compatibilidade para encaminhar chamadas antigas ao `run.sh`.
- [scripts/](scripts): scripts Python e sh de pos-processamento, validacao, plotagem e orquestracao interna.
- [scripts/solver_api.py](scripts/solver_api.py): interface centralizada para chamar `build/goell_q_solver`.
- [scripts/validate_bessel.py](scripts/validate_bessel.py): validacao das funcoes de Bessel contra `scipy.special`.
- [Makefile](Makefile): build alternativo para o solver oficial em `src/goell_q_solver.cpp`.
- [memory/](memory): area de refatoracao modular em C++ ainda experimental; o solver oficial continua em `src/goell_q_solver.cpp`.

### Documentacao

O centro da documentacao agora e [docs/README.md](docs/README.md), que organiza a traducao principal, as notas permanentes de apoio e os links para o PDF.

Trilha principal:

- [docs/00_titulo_resumo.md](docs/00_titulo_resumo.md): titulo, resumo e referencia bibliografica do artigo.
- [docs/01_introducao.md](docs/01_introducao.md): traducao da introducao.
- [docs/02_derivacao_das_equacoes.md](docs/02_derivacao_das_equacoes.md): inicio da Secao II, com expansoes e campos tangenciais.
- [docs/02.1_efeitos_da_simetria.md](docs/02.1_efeitos_da_simetria.md) a [docs/02.7_metodo_de_computacao.md](docs/02.7_metodo_de_computacao.md): desdobramento completo da teoria e do metodo numerico.
- [docs/03_resultados_do_calculo.md](docs/03_resultados_do_calculo.md): abertura da Secao III e Tabela I.
- [docs/03.1_precisao.md](docs/03.1_precisao.md), [docs/03.2_configuracoes_modais.md](docs/03.2_configuracoes_modais.md) e [docs/03.3_curvas_de_propagacao.md](docs/03.3_curvas_de_propagacao.md): resultados, figuras e curvas de propagacao.
- [docs/04_conclusoes.md](docs/04_conclusoes.md), [docs/05_agradecimentos.md](docs/05_agradecimentos.md) e [docs/06_referencias.md](docs/06_referencias.md): fechamento do artigo.

Como referencia rapida:

- [docs/referencias/01_expansoes_de_campo.md](docs/referencias/01_expansoes_de_campo.md)
- [docs/referencias/02_matriz_global_e_normalizacao.md](docs/referencias/02_matriz_global_e_normalizacao.md)
- [docs/referencias/03_checklist_de_conferencia.md](docs/referencias/03_checklist_de_conferencia.md)
- [docs/referencias/04_notas_sobre_a_tabela_1.md](docs/referencias/04_notas_sobre_a_tabela_1.md)

Pendencias do projeto:

- [TODO.md](TODO.md)

### PDF De Referencia

- [docs/j.1538-7305.1969.tb01168.x.pdf](docs/j.1538-7305.1969.tb01168.x.pdf)

## Estado Atual Da Implementacao

O solver e o pipeline de reproducao foram reorganizados para refletir a classificacao correta do artigo:

- paridade dos harmonicos: `odd` e `even`;
- familia de fase: `phi0` e `phi90`.

Isso gera as quatro classes modais tratadas explicitamente no codigo:

- `odd / phi0`
- `odd / phi90`
- `even / phi0`
- `even / phi90`

Tambem ja existe suporte a duas variantes importantes de busca:

- busca por mudanca de sinal de `det(Q)`, que agora e o modo canonico de reproducao;
- busca por minimos de uma metrica escalar, mantida como diagnostico.

No fluxo atual:

- `run.sh` compila automaticamente `build/goell_q_solver` quando necessario;
- a Tabela I usa selecao autonoma do primeiro ramo fisico, sem depender do atalho "nearest paper root";
- as Figs. 16-19 passam por exportacao bruta, filtragem por estabilidade em `N={5,7,9}` e rastreamento por continuidade em `B`;
- o caso `even` com `a/b != 1` usa por padrao a regra de matching descrita no paper, mas aceita uma variante diagnostica `square-split`;
- as Figs. 20-22 agora usam um pipeline separado para os dois modos principais, selecionados como os ramos `odd/phi0` e `odd/phi90` de menor cutoff apos estabilidade em `N`;
- os artefatos finais ficam separados entre `out/` e `figures/`.

## O Que Ja Esta Bem Encaminhado

- a estrutura em quatro classes modais;
- a montagem geral da matriz `Q`;
- a interpretacao da fronteira por intersecao geometrica;
- a reproducao automatizada da Tabela I com meta numerica objetiva;
- o fluxo publico `sh -> C++ -> Python` para as Figs. 16-19;
- o sweep dos modos principais para as Figs. 20-22;
- a exportacao de CSVs brutos, estaveis e rastreados;
- a documentacao das equacoes e da narrativa fisica do artigo.

## O Que Ainda Esta Em Aberto

- tratamento fino do setor `even`, que continua mais sensivel;
- refinamento da identificacao modal final e da leitura fisica das curvas das Figs. 16-19;
- rotulagem fisica final dos modos principais nas Figs. 20-22;
- implementacao futura dos mapas de campo das Figs. 4-15;
- interpretacao final da nota de reescalonamento da p. 2144.

Em outras palavras: o repositorio ja esta num ponto em que a dificuldade principal nao e mais "montar alguma versao do metodo", e sim "fazer a reproducao numerica convergir para os mesmos ramos do artigo".

## Como Compilar

Dependencias principais:

- `g++` com suporte a C++17
- Eigen
- Python 3
- `matplotlib`

Compilacao recomendada:

```bash
./run.sh build
```

Ou, se preferir o build tradicional do repositorio:

```bash
make
```

Compilacao manual, se precisar depurar o ambiente:

```bash
mkdir -p build
g++ -O3 -std=c++17 src/goell_q_solver.cpp -I /usr/include/eigen3 -o build/goell_q_solver
```

## Como Rodar

Fluxo publico principal:

```bash
./run.sh table1
./run.sh fig16
./run.sh fig17
./run.sh fig18
./run.sh fig19
./run.sh fig20
./run.sh fig21
./run.sh fig22
./run.sh validate
./run.sh all-core
./run.sh all-curves
```

`src/presets.sh` continua existindo apenas como compatibilidade para chamadas antigas.

Os CSVs sao escritos em `out/` e as figuras em `figures/`. Para cada figura principal, o fluxo atual produz:

- 4 CSVs brutos por classe modal;
- 4 CSVs estaveis filtrados por consistencia em `N`;
- 4 CSVs rastreados por continuidade em `B`;
- 1 figura de depuracao por classe;
- 1 figura final com as curvas selecionadas;
- 1 resumo Markdown de validacao.

Para os sweeps dos modos principais das Figs. 20-22, o fluxo atual produz:

- CSVs estaveis e rastreados para `odd/phi0` e `odd/phi90` em cada valor do parametro varrido;
- um CSV agregado `out/<tag>_principal_modes.csv`;
- uma figura agregada `figures/<tag>_principal_modes.png`;
- um resumo Markdown `out/<tag>_principal_modes.md`.

Tambem e possivel sobrescrever parametros por variaveis de ambiente:

```bash
RAW_N=7 RAW_NB=80 STABLE_PSCAN=320 ./run.sh fig17
RAW_METRIC=det RAW_DET_SEARCH=sign ./run.sh fig16
FIG_NR=1.0001 FIG_A_OVER_B=2 ./run.sh fig17
TABLE1_MODE=nearest-paper ./run.sh table1
EVEN_RECT_MODE=square-split ./run.sh fig17
FIG20_NR_VALUES=1.0001,1.01,1.1,1.5 ./run.sh fig20
FIG22_ASPECT_VALUES=1,2,4,8 ./run.sh fig22
SWEEP_REUSE_EXISTING=1 ./run.sh fig22
```

`SWEEP_REUSE_EXISTING=1` e util para retomar sweeps longos das Figs. 20-22 sem recalcular valores que ja tenham CSV principal exportado.

## Colunas Exportadas Pelo Solver

O esquema bruto canonico usa:

- `branch_id`
- `B`
- `Pprime`
- `merit`
- `parity`
- `phase`
- `geometry`

Os CSVs estaveis adicionam:

- `support`
- `n_values`

Observacao importante: no estado atual do projeto, `branch_id` ainda nao equivale automaticamente a uma identificacao modal fisica final como a do artigo. Ele deve ser lido como identificador local do candidato exportado ou do ramo rastreado.

## Validacao

Hoje, os testes de validacao mais importantes do repositorio sao:

- reproducao da Tabela I;
- comparacao qualitativa e quantitativa das Figs. 16-19;
- exploracao dos efeitos de `\Delta n_r` e da razao de aspecto para as Figs. 20-22.
- verificacao independente das funcoes de Bessel via `python3 scripts/validate_bessel.py`.

A Tabela I tem sido especialmente util porque ela testa a convergencia em funcao do numero de harmonicos, isto e, testa o proprio coracao numerico do metodo. O alvo operacional atual e:

- `MAE <= 0.01`
- erro absoluto maximo `<= 0.02`

Esses limites ja sao checados diretamente por `./run.sh validate`.

Para rodar todas as curvas de propagacao ja implementadas, use `./run.sh all-curves`. Esse comando cobre Figs. 16-22 e pode ser mais demorado que `validate`.

## Como Ler O Projeto

Se voce estiver chegando agora, a ordem recomendada e:

1. [docs/README.md](docs/README.md)
2. [docs/00_titulo_resumo.md](docs/00_titulo_resumo.md)
3. [docs/01_introducao.md](docs/01_introducao.md)
4. [docs/02_derivacao_das_equacoes.md](docs/02_derivacao_das_equacoes.md)
5. [docs/03_resultados_do_calculo.md](docs/03_resultados_do_calculo.md)
6. [src/goell_q_solver.cpp](src/goell_q_solver.cpp)

Se o objetivo for apenas localizar formulas ou conferir a implementacao:

1. [docs/referencias/01_expansoes_de_campo.md](docs/referencias/01_expansoes_de_campo.md)
2. [docs/referencias/02_matriz_global_e_normalizacao.md](docs/referencias/02_matriz_global_e_normalizacao.md)
3. [src/goell_q_solver.cpp](src/goell_q_solver.cpp)

## Observacao Final

Este README nao tenta mais carregar sozinho a traducao do artigo inteiro. Essa funcao agora foi distribuida entre os arquivos de `docs/`, com navegacao centralizada em [docs/README.md](docs/README.md). O papel do README passou a ser o que ele faz melhor: servir como mapa geral do repositorio, do fluxo numerico e do estado da reproducao.
