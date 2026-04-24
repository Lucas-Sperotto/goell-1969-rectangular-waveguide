# Tabela I - Nota Do Repositorio Sobre A Variacao Residual

_Navegacao: [README geral](../../README.md) | [Indice de docs](../README.md) | [Referencia anterior](03_checklist_de_conferencia.md) | [TODO do projeto](../../TODO.md)_

Este arquivo nao e uma traducao do artigo. Ele registra o que aprendemos ao tentar reproduzir numericamente a Tabela I com o solver deste repositorio.

## O Que A Tabela I Mede No Artigo

Na Secao 3.1, Goell usa a Tabela I para estimar a precisao dos resultados. O experimento e simples:

- fixa-se o primeiro modo em `B = 2`;
- variam-se o numero de harmonicos e a razao de aspecto `a/b`;
- observa-se como a variavel de propagacao normalizada se estabiliza.

O objetivo da tabela nao e classificar todos os modos do problema, mas testar a convergencia do metodo quando o truncamento harmonico muda.

## Melhor Configuracao Encontrada No Repositorio

Entre as variantes testadas, a configuracao que melhor reproduziu a tabela foi:

- `geometry = intersection`
- `metric = det`
- `det-search = sign`
- `rescale = off` ou `on`, com impacto global muito pequeno

Resumo dos erros medidos:

| geometry | det-search | rescale | RMSE | MAE | pior erro |
| :-- | :-- | :-- | --: | --: | --: |
| `intersection` | `sign` | `off` | `0.012603` | `0.009735` | `0.031270` |
| `intersection` | `minima` | `off` | `0.037199` | `0.020985` | `0.111538` |
| `literal` | `sign` | `off` | `0.093350` | `0.072888` | `0.161010` |
| `intersection` | `sign` | `on` | `0.012576` | `0.009676` | `0.031270` |

## Leitura Fisica E Numerica Desses Resultados

Esses numeros sugerem tres coisas importantes.

### 1. A geometria da fronteira importa muito

A interpretacao `intersection` da fronteira radial funciona bem melhor do que a variante `literal`. Isso e coerente com a filosofia do paper: o casamento dos campos so faz sentido se a localizacao dos pontos sobre a borda estiver geometricamente bem representada.

### 2. Procurar raizes reais e melhor do que procurar minimos locais

Buscar mudancas de sinal em `det(Q)` foi muito mais robusto do que simplesmente procurar minimos de `log|det|`. Isso aproxima melhor o fluxo numerico do procedimento descrito no artigo, que procura raizes da condicao modal.

### 3. O reescalonamento nao parece ser o gargalo atual

Ligar ou desligar o reescalonamento da matriz quase nao alterou o erro global da Tabela I. Portanto, a discrepancia residual provavelmente nao esta sendo causada por essa parte do algoritmo.

## O Que Nao Parece Explicar A Variacao

Nos piores casos, aumentar a resolucao da varredura em `P'` de `120` para `800` nao mudou a raiz selecionada. Isso ocorreu, por exemplo, em:

- `a/b = 1`, `N = 7`
- `a/b = 2`, `N = 9`
- `a/b = 3`, `N = 9`
- `a/b = 4`, `N = 7`

Conclusao: a variacao restante nao parece ser mero problema de malha na varredura da raiz.

## Suspeita Principal

O comportamento observado sugere que o problema dominante, hoje, e a identificacao consistente do ramo modal correto quando `N` muda. Em varios casos, a classe selecionada oscila entre combinacoes diferentes de paridade e familia de fase.

Em particular:

- para `a/b = 3`, a selecao pode alternar entre `odd/phi0`, `even/phi90` e `odd/phi90`;
- para `a/b = 4`, ha alternancia entre `odd/phi0` e `odd/phi90`.

Em linguagem menos tecnica: o solver ja encontra varias raizes plausiveis, mas ainda nao faz de modo plenamente estavel a escolha do "mesmo modo fisico" quando o truncamento harmonico muda.

## O Que Isso Ensina

Esta nota tem valor didatico porque mostra que ha duas dificuldades distintas em um problema modal:

1. localizar as raizes da equacao modal;
2. identificar corretamente qual raiz corresponde ao modo que se quer acompanhar.

No estado atual do repositorio, a primeira parte melhorou bastante. A segunda ainda exige trabalho fino de rastreamento modal.

## Proximos Passos Naturais

- melhorar o **branch tracking** com continuidade em `B`;
- revisar com mais cuidado o setor `even`, que continua mais sensivel;
- usar a Tabela I nao apenas como teste de "valor numerico", mas como teste de identificacao modal consistente.

Esta e, portanto, uma nota do repositorio sobre reproducao numerica, e nao parte da traducao do artigo em si.

_Relacionada a: [TODO.md](../../TODO.md) | [Indice de docs](../README.md)_
