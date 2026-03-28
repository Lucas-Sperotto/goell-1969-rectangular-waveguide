# II. Derivacao Das Equacoes - Leitura Guiada Da Secao Teorica

## Como Ler Esta Secao

Esta e a parte mais importante do artigo. E aqui que Goell transforma um problema fisico de propagacao guiada em um problema numerico de algebra linear.

Se eu tivesse de resumir a secao inteira em um unico diagrama mental, seria este:

1. escolher uma expansao adequada para os campos;
2. usar simetria para reduzir o problema;
3. impor as condicoes de contorno em pontos da fronteira;
4. montar uma matriz `Q`;
5. procurar os valores para os quais `det(Q) = 0`.

Cada subsecao abaixo explica um desses passos.

## 2.0 Geometria E Convencoes

O guia de onda considerado por Goell tem:

- um nucleo dieletrico retangular;
- permissividade `\epsilon_1` no interior;
- meio externo infinito com permissividade `\epsilon_0`;
- permeabilidade `\mu_0` em ambas as regioes;
- propagacao ao longo de `+z`.

As dimensoes completas da secao transversal sao:

- `a` na direcao `x`;
- `b` na direcao `y`.

O nucleo fica centrado na origem, de modo que, no primeiro quadrante, o canto do retangulo esta em

$$
\left(\frac{a}{2}, \frac{b}{2}\right).
$$

O angulo radial que enxerga esse canto e

$$
\theta_c = \tan^{-1} \left(\frac{b}{a}\right).
$$

Comentario didatico: o artigo usa ao mesmo tempo coordenadas cartesianas e cilindricas. A fronteira fisica e retangular, mas a base funcional escolhida para os campos e circular. Essa tensao entre geometria da estrutura e geometria da expansao e o coracao do metodo.

## 2.1 Expansao Dos Campos Longitudinais

Goell parte das componentes longitudinais `E_z` e `H_z`, porque, em meios isotropicos, elas bastam para reconstruir as componentes transversais.

### Dentro do nucleo

As componentes longitudinais no interior sao escritas como

$$
E_{z1} = \sum_{n=0}^{\infty} a_n J_n(h r)\sin(n\theta + \phi_n) e^{i(k_z z - \omega t)},
$$

$$
H_{z1} = \sum_{n=0}^{\infty} b_n J_n(h r) \sin(n\theta + \psi_n) e^{i(k_z z - \omega t)}.
$$

### Fora do nucleo

No meio externo, as expansoes passam a ser

$$
E_{z0} = \sum_{n=0}^{\infty} c_n K_n(p r) \sin(n\theta + \phi_n) e^{i(k_z z - \omega t)},
$$

$$
H_{z0} = \sum_{n=0}^{\infty} d_n K_n(p r) \sin(n\theta + \psi_n) e^{i(k_z z - \omega t)}.
$$

As constantes transversais sao

$$
h = \sqrt{k_1^2 - k_z^2},
\qquad
p = \sqrt{k_z^2 - k_0^2},
$$

com

$$
k_1 = \omega \sqrt{\mu_0 \epsilon_1},
\qquad
k_0 = \omega \sqrt{\mu_0 \epsilon_0}.
$$

### O Significado Fisico Dessa Escolha

- `J_n(hr)` aparece no interior porque a solucao precisa ser regular na origem.
- `K_n(pr)` aparece no exterior porque a solucao precisa decair quando `r \to \infty`.

Comentario didatico: este e o primeiro grande acerto do metodo. O artigo nao escolhe funcoes bonitas; escolhe funcoes que respeitam a fisica radial do problema.

## 2.2 Campos Transversais A Partir De `E_z` E `H_z`

As componentes transversais sao obtidas das equacoes de Maxwell. O artigo escreve

$$
E_r = \frac{i k_z}{k^2 - k_z^2} \left[ \frac{\partial E_z}{\partial r} + (\frac{\mu_0 \omega} {k_z r}) \frac{\partial H_z}{\partial \theta} \right],
$$

$$
E_{\theta} =
\frac{i k_z}{k^2 - k_z^2}
\left[
\frac{1}{r}\frac{\partial E_z}{\partial \theta}
-
\frac{\mu_0 \omega}{k_z}
\frac{\partial H_z}{\partial r}
\right],
$$

$$
H_r =
\frac{i k_z}{k^2 - k_z^2}
\left[
-
(\frac{k^2}{\mu_0 \omega k_z r})
\frac{\partial E_z}{\partial \theta}
+
\frac{\partial H_z}{\partial r}
\right],
$$

$$
H_{\theta} =
\frac{i k_z}{k^2 - k_z^2}
\left[
(\frac{k^2}{\mu_0 \omega k_z})
\frac{\partial E_z}{\partial r}
+
\frac{1}{r}\frac{\partial H_z}{\partial \theta}
\right].
$$

Aqui `k` representa `k_1` no interior e `k_0` no exterior.

Comentario didatico: o artigo nao resolve um problema escalar. Os campos transversais misturam derivadas radiais e angulares de `E_z` e `H_z`. E exatamente essa mistura que acaba aparecendo nos blocos tangenciais da matriz.

## 2.3 Campo Tangencial Na Fronteira Retangular

O casamento de contorno nao e imposto em `E_r` e `E_{\theta}` isoladamente, mas no campo tangencial a fronteira do retangulo.

Nos lados verticais, o artigo escreve

$$
E_t =
\pm \left(E_r \sin\theta + E_{\theta}\cos\theta\right),
$$

para os intervalos angulares que enxergam esses lados.

Nos lados horizontais,

$$
E_t =
\pm \left(-E_r \cos\theta + E_{\theta}\sin\theta\right).
$$

Expressoes analogas valem para `H_t`.

Comentario didatico: aqui a geometria do retangulo entra explicitamente. A base e circular, mas o campo tangencial e projetado na normal e na tangente corretas de cada trecho da fronteira.

## 2.4 Efeitos Da Simetria

Esta e uma das partes mais elegantes do artigo, porque ela reduz o problema sem perder fisica.

### Simetria em relacao ao eixo `x`

Goell mostra que existem duas familias de fase:

1. primeira familia:

$$
\phi_n = 0,
\qquad
\psi_n = \frac{\pi}{2},
$$

2. segunda familia:

$$
\phi_n = \frac{\pi}{2},
\qquad
\psi_n = \pi.
$$

No repositorio, essas duas familias aparecem como:

- `phi0`
- `phi90`

### Simetria em relacao ao eixo `y`

O artigo reescreve uma das expansoes em torno de `\alpha = \theta - \pi/2` e conclui que um modo fisico completo deve usar:

- apenas harmonicos impares, ou
- apenas harmonicos pares.

No repositorio, isso aparece como:

- `odd`
- `even`

### O Resultado Pratico

Em vez de um solver unico e cego, passamos a ter classes modais separadas por:

- familia de fase;
- paridade dos harmonicos.

Comentario didatico: simetria aqui nao e um detalhe estetico. Ela reduz o tamanho da base, o numero de pontos de casamento e o risco de misturar modos de classes diferentes.

## 2.5 Escolha Dos Pontos De Casamento

Os pontos de casamento devem ser distribuidos simetricamente. Como a estrutura e simetrica em ambos os eixos, basta trabalhar em um quadrante.

### Casos de harmonicos impares

Para os casos `odd`, os pontos usados por Goell sao

$$
\theta_m =
\frac{(m - 1/2)\pi}{2N},
\qquad
m = 1,2,\ldots,N,
$$

onde `N` e o numero de harmonicos espaciais mantidos no truncamento.

### Casos de harmonicos pares

Aqui a situacao fica mais delicada, porque o termo `n = 0` nao pode ser admitido simultaneamente de forma ingenua em todos os blocos, sob pena de violar a simetria do problema.

Por isso:

- o numero total de incognitas passa de `4N` para `4N - 2`;
- os pontos de casamento dependem do tipo de simetria da componente considerada.

Para `a/b = 1`, o artigo usa:

- para componentes com simetria par em torno de `\theta = 0`,

$$
\theta_m =
\frac{(m - 1/2)\pi}{2N},
\qquad
m = 1,\ldots,N;
$$

- para componentes com simetria impar em torno de `\theta = 0`,

$$
\theta_m =
\frac{(m - N - 1/2)\pi}{2(N-1)},
\qquad
m = N+1,\ldots,2N-1.
$$

Para `a/b \neq 1`, Goell diz que todos os pontos seguem a primeira formula, exceto os primeiros e ultimos pontos do componente `z` impar, que sao omitidos.

Comentario didatico: esta regra especial do caso `even` e uma das fontes mais provaveis de erro de implementacao. Vale sempre reler essa parte ao comparar o solver com o paper.

## 2.6 Formulacao Matricial

As condicoes de contorno sao escritas em forma matricial. O artigo separa primeiro:

- casamento longitudinal de `E_z`;
- casamento longitudinal de `H_z`;
- casamento tangencial de `E_t`;
- casamento tangencial de `H_t`.

As equacoes sao

$$
E^{LA} A = E^{LC} C,
$$

$$
H^{LB} B = H^{LD} D,
$$

$$
E^{TA} A + E^{TB} B = E^{TC} C + E^{TD} D,
$$

$$
H^{TA} A + H^{TB} B = H^{TC} C + H^{TD} D.
$$

Aqui `A`, `B`, `C` e `D` sao vetores-coluna contendo os coeficientes modais:

- `a_n`
- `b_n`
- `c_n`
- `d_n`

respectivamente.

## 2.7 Elementos Das Matrizes - Escritos De Forma Didatica

O scan do artigo e pouco amigavel na distincao entre barras e primas. Para fins de estudo, vale separar os termos em dois tipos:

- termos "radiais", vindos de derivadas radiais;
- termos "angulares", vindos dos fatores `n/r`.

Adotaremos a notacao:

$$
S = \sin(n\theta_m + \phi),
\qquad
C = \cos(n\theta_m + \phi),
\qquad
\phi \in \left\{0,\frac{\pi}{2}\right\},
$$

$$
J = J_n(h r_m),
\qquad
K = K_n(p r_m),
$$

$$
J_r = \frac{J_n'(h r_m)}{h},
\qquad
K_r = \frac{K_n'(p r_m)}{p},
$$

$$
J_{\theta} = \frac{n J_n(h r_m)}{h^2 r_m},
\qquad
K_{\theta} = \frac{n K_n(p r_m)}{p^2 r_m},
$$

$$
Z_0 = \sqrt{\frac{\mu_0}{\epsilon_0}},
\qquad
\epsilon_r = \frac{\epsilon_1}{\epsilon_0}.
$$

Com essa escolha, os blocos longitudinais ficam simples:

$$
e_{mn}^{LA} = J S,
\qquad
e_{mn}^{LC} = K S,
$$

$$
h_{mn}^{LB} = J C,
\qquad
h_{mn}^{LD} = K C.
$$

Ja os blocos tangenciais ficam

$$
e_{mn}^{TA} =
-k_z \left(J_r S R + J_{\theta} C T\right),
$$

$$
e_{mn}^{TB} =
k_0 Z_0 \left(J_{\theta} S R + J_r C T\right),
$$

$$
e_{mn}^{TC} =
k_z \left(K_r S R + K_{\theta} C T\right),
$$

$$
e_{mn}^{TD} =
-k_0 Z_0 \left(K_{\theta} S R + K_r C T\right),
$$

$$
h_{mn}^{TA} =
\frac{\epsilon_r k_0}{Z_0}
\left(J_{\theta} C R - J_r S T\right),
$$

$$
h_{mn}^{TB} =
-k_z \left(J_r C R - J_{\theta} S T\right),
$$

$$
h_{mn}^{TC} =
-\frac{k_0}{Z_0}
\left(K_{\theta} C R - K_r S T\right),
$$

$$
h_{mn}^{TD} =
k_z \left(K_r C R - K_{\theta} S T\right).
$$

### Geometria local da fronteira

Os fatores `R`, `T` e `r_m` dependem de qual lado da fronteira e atingido pelo raio de angulo `\theta_m`.

Para `\theta_m < \theta_c`,

$$
R = \sin\theta_m,
\qquad
T = \cos\theta_m,
\qquad
r_m = \frac{a/2}{\cos\theta_m}.
$$

Para `\theta_m > \theta_c`,

$$
R = -\cos\theta_m,
\qquad
T = \sin\theta_m,
\qquad
r_m = \frac{b/2}{\sin\theta_m}.
$$

Comentario didatico: esta escrita para `r_m` segue a interpretacao geometrica da intersecao do raio com a fronteira. Em scans e OCR, essa parte costuma aparecer truncada; por isso vale sempre compara-la com a implementacao real do solver.

## 2.8 Designacao Dos Modos

Guias dielectricos retangulares nao admitem, em geral, uma classificacao pura em TE e TM. Os modos sao mistos.

Goell adota a nomenclatura `E_{mn}` e `H_{mn}` com base no limite de pequeno contraste de indice:

- `E_{mn}` quando o campo eletrico transversal dominante tende a alinhar-se com `y`;
- `H_{mn}` quando tende a alinhar-se com `x`.

Os indices `m` e `n` contam o numero de maximos aproximadamente visiveis nas direcoes `x` e `y`.

Comentario didatico: essa classificacao e util como rotulo fisico, mas nao deve ser confundida com um teorema de separacao estrita. Em guias dielectricos, os modos sao hibridos.

## 2.9 Diferencas Entre Campos Eletrico E Magnetico

O artigo chama a atencao para um contraste com guias metalicos: em um guia dieletrico, os campos transversais `E_t` e `H_t` nao precisam ser ortogonais ponto a ponto.

A condicao de ortogonalidade local seria

$$
E_t \cdot H_t = E_r H_r + E_{\theta} H_{\theta} = 0.
$$

Mas, usando as expressoes dos campos transversais, Goell mostra que

$$
E_t \cdot H_t =
\frac{k^2 - k_z^2}{k_z^2}
\left(
\frac{\partial H_z}{\partial r}\frac{\partial E_z}{\partial r}
+
\frac{1}{r^2}
\frac{\partial H_z}{\partial \theta}
\frac{\partial E_z}{\partial \theta}
\right).
$$

Logo:

- a impedancia transversal depende da posicao;
- pode haver mudanca de sinal do fluxo de potencia;
- os campos eletrico e magnetico nao sao "copias um do outro" como em guias metalicos ideais.

Comentario didatico: esse ponto ajuda a entender por que os desenhos de linhas de campo e os padroes de intensidade do artigo sao mais ricos do que os de um guia metalico retangular.

## 2.10 Normalizacao

Goell introduz variaveis normalizadas para que as curvas de propagacao fiquem pouco sensiveis ao contraste de indice quando `\Delta n_r` e pequeno.

O artigo escreve a variavel de propagacao normalizada como

$$
P^2_{paper} =
\frac{(k_z/k_0)^2 - 1}{n_r^2 - 1},
$$

e define ainda uma quantidade radial normalizada

$$
\Omega = r k_0 \sqrt{n_r^2 - 1}.
$$

$$
n_r = \sqrt{(k_1/k_0)}.
$$

Com isso, os argumentos das funcoes especiais podem ser reescritos como

$$
p r = P_{paper}\,\Omega,
$$

$$
h r = \Omega \sqrt{1 - P^2_{paper}}.
$$

Para as curvas, a variavel horizontal usada pelo artigo e

$$
B_{paper} =
\frac{2b}{\lambda_0}\sqrt{n_r^2 - 1},
\qquad
\lambda_0 = \frac{2\pi}{k_0}.
$$

No limite de pequena diferenca de indice, o artigo observa que a variavel vertical fica aproximadamente proporcional ao excesso de constante de propagacao em relacao ao meio externo. Em termos fisicos, isso faz dela uma excelente coordenada para visualizar o afastamento do modo em relacao ao corte.

Comentario didatico: no codigo do repositorio, essas quantidades aparecem sobretudo como `B` e `Pprime`. O importante e saber que elas sao variaveis adimensionais construidas para tornar as curvas comparaveis entre diferentes guias.

## 2.11 Equacao Matricial Global E Condicao Modal

As quatro familias de equacoes de contorno sao reunidas em uma unica equacao matricial

$$
[Q][T] = 0,
$$

com

$$
Q =
\begin{bmatrix}
E^{LA} & 0      & -E^{LC} & 0 \\
0      & H^{LB} & 0       & -H^{LD} \\
E^{TA} & E^{TB} & -E^{TC} & -E^{TD} \\
H^{TA} & H^{TB} & -H^{TC} & -H^{TD}
\end{bmatrix},
$$

e

$$
[T] =
\begin{bmatrix}
A \\
B \\
C \\
D
\end{bmatrix}.
$$

Para existir uma solucao nao trivial, e necessario que

$$
\det(Q) = 0.
$$

Esta e a equacao modal do artigo.

Comentario didatico: o problema inteiro foi reduzido a "para quais valores do parametro de propagacao essa matriz perde posto?". Essa e a pergunta numerica que o solver implementa.

## 2.12 Metodo De Computacao

Goell descreve um fluxo computacional muito claro.

### Para encontrar a constante de propagacao

1. amostrar valores-teste da variavel normalizada em `(0,1)`;
2. localizar aproximadamente as raizes de `det(Q)`;
3. refinar as raizes pelo metodo de Newton.

O artigo comenta que, em geral:

- uma iteracao de Newton bastava para curvas de propagacao;
- cerca de dez iteracoes eram usadas quando se desejavam padroes de campo mais precisos.

### Para avaliar o determinante

Foram usados:

- triangulacao simples, na maior parte dos casos;
- condensacao por pivoteamento de Gauss, quando o arredondamento se tornava preocupante.

### Para controlar a faixa dinamica numerica

O artigo comenta que linhas e colunas da matriz podiam ser reescaladas por fatores positivos sem deslocar os zeros do determinante. Essa observacao e teoricamente correta e continua sendo relevante em implementacoes modernas.

Observacao do repositorio: a passagem da pagina 2144 que descreve o reescalonamento exato continua sendo a parte menos nitida do scan. A ideia fisica, no entanto, e clara: manter os blocos da matriz em uma faixa numerica razoavel sem alterar os zeros de `det(Q)`.

## 2.13 Como Os Padroes De Campo Sao Reconstruidos

Depois que a raiz modal e encontrada:

1. substitui-se esse valor em `Q`;
2. fixa-se um coeficiente do vetor `T`;
3. resolvem-se os demais coeficientes por algebra linear;
4. reconstrui-se o campo em uma malha do plano transversal.

O artigo descreve entao:

- cortes radiais;
- isolinhas;
- figuras de intensidade;
- desenhos de linhas de campo.

Comentario didatico: essa parte e importante porque mostra que o metodo nao entrega apenas "um numero". Ele entrega tambem a estrutura espacial do modo, isto e, a parte mais fisicamente rica do problema.

## O Que O Aluno Deve Guardar Desta Secao

- O metodo usa `E_z` e `H_z` como variaveis geradoras.
- A base radial e composta por Bessel no nucleo e Bessel modificada no exterior.
- A simetria reduz o problema a classes `odd/even` e `phi0/phi90`.
- A fronteira retangular entra pelos pontos de casamento.
- O resultado final e uma matriz `Q` cujo determinante deve se anular.
- As curvas de propagacao e os padroes de campo saem da mesma estrutura matematica.

Em outras palavras: a Secao II do artigo nao e apenas "a derivacao". Ela e o projeto completo do solver.
