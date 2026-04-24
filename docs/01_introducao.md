# 1. Introdução

_Navegacao: [README geral](../README.md) | [Indice de docs](README.md) | [Anterior: Titulo e resumo](00_titulo_resumo.md) | [Proximo: 2. Derivacao das equacoes](02_derivacao_das_equacoes.md)_

Prevê-se que os guias de onda dielétricos sejam utilizados como blocos fundamentais na construção de circuitos ópticos integrados. Esses guias podem servir não apenas como meio de transmissão para confinar e direcionar sinais ópticos, mas também como base para circuitos tais como filtros e acopladores direcionais. Assim, é importante possuir um conhecimento aprofundado das propriedades de seus modos.

Os guias de onda dielétricos circulares têm recebido considerável atenção, porque a geometria circular é comumente empregada em fibras ópticas. Em muitas aplicações de óptica integrada, espera-se que os guias de onda consistam em um núcleo dielétrico retangular, ou aproximadamente retangular, embebido em um meio dielétrico de índice de refração ligeiramente menor. Os modos dessa geometria são mais difíceis de analisar do que os do guia de onda retangular metálico, devido à natureza da fronteira.

Marcatili, utilizando aproximações baseadas na hipótese de que a maior parte do fluxo de potência está confinada ao núcleo do guia de onda, derivou em forma fechada as propriedades de um guia de onda dielétrico retangular. Em sua solução, campos com variação senoidal no núcleo são ajustados a campos exponencialmente decrescentes no meio externo. Em cada região, apenas um único modo é utilizado. Os resultados desse método são obtidos em uma forma relativamente simples para avaliação numérica.

As propriedades do modo principal do guia de onda dielétrico retangular foram estudadas por Schlosser e Unger com o uso de um computador digital de alta velocidade. Na abordagem deles, o plano transversal foi dividido em regiões, como mostrado na Figura 1, e soluções em coordenadas retangulares foram assumidas em cada uma dessas regiões. A constante de propagação longitudinal era então ajustada para que fosse possível obter compatibilidade de campo em pontos discretos ao longo da fronteira. Esse método fornece resultados que, teoricamente, são válidos em uma faixa mais ampla do que a de Marcatili, mas com um aumento significativo da dificuldade computacional. Uma limitação do método é que, para um dado modo, à medida que o comprimento de onda aumenta, a extensão espacial do campo também aumenta; assim, no limite, torna-se cada vez mais difícil ajustar os campos ao longo das fronteiras entre as regiões [1] e [2] e entre as regiões [2] e [3].

![Fronteiras de acoplamento para a análise de modos em guias de onda retangulares.](img/fig_01.png)

Figura 1 — Fronteiras de acoplamento para a análise de modos em guias de onda retangulares.

Uma abordagem variacional foi desenvolvida por Shaw e outros autores. Eles assumem uma solução de teste com dois ou três parâmetros variáveis no núcleo. A partir dessa solução de teste, os campos externos ao núcleo são então derivados, e os parâmetros são variados até que se obtenha uma solução consistente. Essa abordagem, assim como a de Schlosser, exige cálculos elaborados. Além disso, apresenta a desvantagem de exigir que a função de teste seja assumida previamente. Soma-se a isso o fato de que alguns de seus resultados preliminares não exibem o comportamento correto nos casos-limite, isto é, quando as dimensões do guia de onda são muito grandes ou muito pequenas em comparação com o comprimento de onda.

Na análise aqui apresentada, a variação radial dos campos elétrico e magnético longitudinais dos modos é representada por uma soma de funções de Bessel no interior do núcleo do guia de onda e por uma soma de funções de Bessel modificadas no exterior do núcleo. As soluções são obtidas ajustando-se os campos ao longo do perímetro do núcleo. Dessa forma, a fronteira de ajuste não depende dos parâmetros do guia de onda, de modo que a complexidade computacional não aumenta com o comprimento de onda.

A Seção II discute a teoria subjacente à análise por harmônicos circulares de guias de onda dielétricos retangulares. Em seguida, apresenta-se uma descrição das técnicas computacionais e dos métodos gráficos especiais de apresentação utilizados. A Seção III está dividida em três partes: a primeira descreve a precisão dos cálculos; a segunda descreve os padrões de campo; e a terceira apresenta curvas de propagação.

_Continua em: [2. Derivacao das equacoes](02_derivacao_das_equacoes.md) | [Indice de docs](README.md)_
