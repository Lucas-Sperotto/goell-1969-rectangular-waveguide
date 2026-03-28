# Handoff Do Projeto - Goell CHM

Este arquivo e uma memoria de continuidade do projeto. A ideia e simples: quando este repositorio for retomado no futuro, ele deve permitir recomecar com contexto tecnico suficiente, sem depender da memoria de curto prazo de quem estava mexendo nele.

Data desta parada: 2026-03-28.

## 1. O Que Este Repositorio Ja Tem De Solido

### Documentacao

O diretorio `docs/` foi reorganizado como trilha de estudo do artigo:

- [00_resumo.md](./00_resumo.md)
- [01_intro.md](./01_intro.md)
- [02_deriv.md](./02_deriv.md)
- [03_result.md](./03_result.md)
- [04_conclusions.md](./04_conclusions.md)

Tambem existem notas compactas de referencia:

- [goell_01_field_expansions.md](./goell_01_field_expansions.md)
- [goell_02_matrix_and_normalization.md](./goell_02_matrix_and_normalization.md)
- [goell_03_review_checklist.md](./goell_03_review_checklist.md)
- [table1_variation_notes.md](./table1_variation_notes.md)

O PDF de referencia foi movido para:

- [j.1538-7305.1969.tb01168.x.pdf](./j.1538-7305.1969.tb01168.x.pdf)

### Implementacao

O solver principal esta em:

- `src/goell_q_solver.cpp`

Pontos importantes da organizacao atual:

- o problema foi separado em quatro classes modais:
  - `odd / phi0`
  - `odd / phi90`
  - `even / phi0`
  - `even / phi90`
- a montagem da matriz `Q` esta estruturada para seguir a Secao II do paper;
- existe busca por:
  - minimos locais de uma metrica;
  - mudanca de sinal de `det(Q)`, mais fiel a eq. (19).

Scripts auxiliares relevantes:

- `src/presets.sh`
- `src/plot_compare.py`
- `src/reproduce_table1.py`
- `src/analyze_table1_variation.py`
- `src/validate_goell.py`
- `src/track_roots.py`

## 2. Estado Atual Da Reproducao

### O que parece bem encaminhado

- a separacao entre paridade e familia de fase;
- a existencia das quatro classes corretas do metodo;
- a estrutura geral de montagem da matriz `Q`;
- a leitura da fronteira por intersecao geometrica (`geometry=intersection`);
- a reproducao automatizada da Tabela I como ferramenta diagnostica;
- a trilha de documentacao em portugues.

### O que ainda NAO esta validado

- a identificacao modal final dos ramos fisicos nas Figs. 16-22;
- a reproducao visual confiavel das curvas publicadas;
- a convergencia do setor `even`;
- a interpretacao final da nota de reescalonamento da p. 2144;
- a Tabela I como validacao independente do "primeiro modo".

Em resumo: a teoria ja esta bem melhor estruturada do que antes, mas a reproducao numerica ainda nao pode ser considerada encerrada.

## 3. Principais Achados Tecnicos Ate Aqui

### Achado A - bloco `H^{TA}` provavelmente assimetrico

No solver atual, o bloco associado a `H^{TA}` usa `JprimeRaw = J'_n(hr)` sem a divisao por `h`, enquanto os outros blocos tangenciais usam formas escaladas (`JbarPrime`, `KbarPrime` etc.).

Suspeita:

- isso pode estar alterando a ponderacao relativa do setor magnetico tangencial;
- o efeito tende a ser mais sensivel perto do corte, onde `h` fica pequeno;
- esse ponto merece ser reaberto com o paper lado a lado.

Arquivos relevantes:

- `src/goell_q_solver.cpp`
- `docs/goell_02_matrix_and_normalization.md`

### Achado B - `safe_positive()` esta entrando nos argumentos fisicos

O helper `safe_positive()` nao esta sendo usado apenas para evitar divisao por zero em denominadores. Ele tambem entra nos argumentos `hr` e `pr` das funcoes de Bessel.

Suspeita:

- isso altera a equacao modal efetivamente resolvida perto de `B ~ 0`, `P' ~ 0` e `P' ~ 1`;
- pode introduzir vies nas bordas justamente onde o solver mais sofre com ramos espurios;
- esse comportamento deve ser separado em:
  - protecao numerica para denominadores;
  - avaliacao fisica correta dos argumentos das funcoes especiais.

### Achado C - `det-search=sign` e util, mas incompleto

A busca por mudanca de sinal de `det(Q)` foi um avanco real, porque aproxima melhor o algoritmo do procedimento descrito no artigo.

Mas ela perde:

- raizes de multiplicidade par;
- toques tangenciais em zero;
- quase-degenerescencias que nao mudam o sinal entre duas amostras vizinhas.

Portanto, ela nao deve ser tratada como oraculo final. Ela e uma ferramenta boa, mas nao suficiente sozinha.

### Achado D - Tabela I ainda nao e validacao independente

O script `src/reproduce_table1.py` ainda escolhe, para cada caso, a raiz calculada mais proxima do valor publicado no paper.

Isso foi mantido por honestidade metodologica e transparencia, mas significa que:

- a Tabela I atual e uma ferramenta de conferencia;
- ela ainda nao prova que o solver esta identificando sozinho o primeiro modo correto em cada caso.

## 4. O Que A Validacao Ja Mostrou

### Tabela I

Melhor configuracao encontrada ate agora:

- `geometry = intersection`
- `metric = det`
- `det-search = sign`
- `rescale = off` ou `on` com efeito global pequeno

Interpretacao atual:

- a geometria da fronteira importa muito;
- procurar raizes reais e melhor do que procurar apenas vales de `log|det|`;
- a variacao residual parece vir mais de identificacao modal instavel do que de resolucao da varredura.

### Figuras 16-19

Leitura atual do estado do projeto:

- `fig17` melhorou quando passamos a usar `n_r = 1.0001` como aproximacao pratica de `\Delta n_r \to 0`;
- `fig16` continua sendo o caso mais problematico;
- os ramos `odd` parecem mais coerentes;
- o setor `even` continua muito povoado e mais sensivel.

### Figuras 20-22

Ainda nao estao em estado de validacao final.

Motivo:

- antes de atacar esses casos, e preciso estabilizar melhor a identificacao dos ramos fisicos nas Figs. 16-19.

## 5. Inconsistencias De Configuracao Que Devem Ser Lembradas

Hoje ha uma divergencia entre o preset rapido e o helper de validacao:

- `src/presets.sh` usa `n_r = 1.01` como padrao para `fig16` e `fig17`;
- `src/validate_goell.py` usa `n_r = 1.0001` como aproximacao de `\Delta n_r \to 0`.

Isso nao e necessariamente "erro", mas precisa ser lembrado sempre que resultados diferentes forem comparados.

Recomendacao de retomada:

- consolidar explicitamente quando usar `1.01`;
- consolidar explicitamente quando usar `1.0001`;
- evitar comparar figuras geradas em regimes diferentes sem dizer isso no texto.

## 6. Ordem Recomendada Para Retomar O Projeto

Quando este repositorio for retomado, a ordem sugerida e:

1. ler [00_resumo.md](./00_resumo.md), [02_deriv.md](./02_deriv.md) e este arquivo;
2. reler `src/goell_q_solver.cpp` apenas nos blocos tangenciais e na geometria da fronteira;
3. testar especificamente:
   - o bloco `H^{TA}`;
   - o uso de `safe_positive()` nos argumentos `hr/pr`;
4. rerodar a Tabela I;
5. rerodar `fig16` e `fig17` com configuracao documentada e consistente;
6. so depois voltar a `fig18-22`.

Se houver pouco tempo no retorno, o melhor ataque inicial e:

- revisar `H^{TA}`;
- separar protecao numerica de alteracao fisica em `hr/pr`;
- repetir a Tabela I.

## 7. Comandos Uteis Para Retomada

### Compilar

```bash
mkdir -p build
g++ -O3 -std=c++17 src/goell_q_solver.cpp -I /usr/include/eigen3 -o build/goell_q_solver
```

### Rodar figuras principais

```bash
bash src/presets.sh fig16
bash src/presets.sh fig17
bash src/presets.sh fig18
bash src/presets.sh fig19
```

### Reproduzir a Tabela I

```bash
python3 src/reproduce_table1.py
```

### Diagnosticar variacao residual da Tabela I

```bash
python3 src/analyze_table1_variation.py
```

### Rodar filtro de estabilidade em `N`

```bash
python3 src/validate_goell.py --stability-figures fig16 fig17 --det-search sign
```

### Rastrear ramos

```bash
python3 src/track_roots.py out/fig16_odd_phi0.csv out/fig16_even_phi0.csv
```

## 8. Ultima Verificacao Antes Da Parada

Nesta ultima passada, antes de encerrar o trabalho:

- o worktree ficou limpo;
- o solver C++ compilou;
- os scripts Python passaram em `py_compile`;
- a documentacao foi reorganizada e commitada.

Observacao pratica:

- os diretorios `out/` e `figures/` nao estao presentes neste snapshot, entao devem ser regenerados normalmente quando o projeto for retomado.

## 9. Commits Relevantes Para Relembrar

Historico recente mais importante:

- `f133db1` - reescrita das notas de estudo e atualizacao do `README`
- `2bb4558` - analise das fontes de variacao da Tabela I
- `d428905` - ferramental de validacao por determinante
- `3e4276f` - modo de validacao de geometria da fronteira
- `a8b3b8f` - refinamento do fluxo de busca por determinante

Ao voltar para o projeto, estes commits sao bons pontos de leitura para entender a linha de evolucao.

## 10. Mensagem Final Para A Proxima Retomada

Se este repositorio for retomado daqui a meses, a situacao deve ser lida assim:

- a parte de estudo do artigo esta em boa forma;
- a arquitetura geral do solver esta muito melhor do que no inicio;
- o problema remanescente e fino e interessante: identificar com fidelidade os ramos modais do paper sem introduzir artificios numericos indevidos.

Ou seja: o projeto nao esta "quebrado". Ele esta numa fase de depuracao fisica e numerica fina, que so faz sentido depois que a estrutura teorica principal ja foi montada.
