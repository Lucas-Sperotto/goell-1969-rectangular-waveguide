# Goell 1969 - Expansoes De Campo, Simetria E Pontos De Casamento

_Navegacao: [README geral](../../README.md) | [Indice de docs](../README.md) | [Proxima referencia](02_matriz_global_e_normalizacao.md)_

Este arquivo e uma folha de referencia rapida para as Secoes 2.1 e 2.2 do artigo. A ideia nao e substituir a leitura principal de [../02_derivacao_das_equacoes.md](../02_derivacao_das_equacoes.md), mas reunir em um so lugar as expressoes que entram diretamente na implementacao.

## 1. Geometria

O nucleo e retangular, centrado na origem, com dimensoes completas:

- `a` na direcao `x`;
- `b` na direcao `y`.

No primeiro quadrante, o canto esta em

$$
\left(\frac{a}{2},\frac{b}{2}\right),
$$

e o angulo do canto e

$$
\theta_c = \tan^{-1}\!\left(\frac{b}{a}\right).
$$

## 2. Expansoes Dos Campos Longitudinais

### Interior do nucleo

$$
E_{z1} =
\sum_{n=0}^{\infty}
a_n J_n(h r)\,\sin(n\theta + \phi_n)\,
e^{i(k_z z - \omega t)},
$$

$$
H_{z1} =
\sum_{n=0}^{\infty}
b_n J_n(h r)\,\sin(n\theta + \psi_n)\,
e^{i(k_z z - \omega t)}.
$$

### Exterior do nucleo

$$
E_{z0} =
\sum_{n=0}^{\infty}
c_n K_n(p r)\,\sin(n\theta + \phi_n)\,
e^{i(k_z z - \omega t)},
$$

$$
H_{z0} =
\sum_{n=0}^{\infty}
d_n K_n(p r)\,\sin(n\theta + \psi_n)\,
e^{i(k_z z - \omega t)}.
$$

## 3. Numeros De Onda Radiais

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

Leitura fisica:

- `J_n` e regular na origem e por isso serve ao interior;
- `K_n` decai no infinito e por isso serve ao exterior.

## 4. Componentes Transversais

As componentes transversais sao obtidas a partir de `E_z` e `H_z`:

$$
E_r =
\frac{i k_z}{k^2 - k_z^2}
\left[
\frac{\partial E_z}{\partial r}
+
\frac{\mu_0 \omega}{k_z r}
\frac{\partial H_z}{\partial \theta}
\right],
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
\frac{k^2}{\mu_0 \omega k_z r}
\frac{\partial E_z}{\partial \theta}
+
\frac{\partial H_z}{\partial r}
\right],
$$

$$
H_{\theta} =
\frac{i k_z}{k^2 - k_z^2}
\left[
\frac{k^2}{\mu_0 \omega k_z}
\frac{\partial E_z}{\partial r}
+
\frac{1}{r}\frac{\partial H_z}{\partial \theta}
\right].
$$

Aqui `k` vale `k_1` no interior e `k_0` no exterior.

## 5. Campo Tangencial Na Fronteira

Nos trechos que correspondem aos lados verticais do retangulo:

$$
E_t = \pm(E_r \sin\theta + E_{\theta}\cos\theta).
$$

Nos trechos que correspondem aos lados horizontais:

$$
E_t = \pm(-E_r \cos\theta + E_{\theta}\sin\theta).
$$

O mesmo raciocinio vale para `H_t`.

## 6. Familias De Fase

O artigo identifica duas familias de fase:

1. primeira familia:

$$
\phi_n = 0,
\qquad
\psi_n = \frac{\pi}{2};
$$

2. segunda familia:

$$
\phi_n = \frac{\pi}{2},
\qquad
\psi_n = \pi.
$$

Na pratica, isso pode ser condensado em um unico parametro `\phi`:

$$
S = \sin(n\theta_m + \phi),
\qquad
C = \cos(n\theta_m + \phi),
\qquad
\phi \in \left\{0,\frac{\pi}{2}\right\}.
$$

No codigo, essas familias aparecem como `phi0` e `phi90`.

## 7. Paridade Dos Harmonicos

Pela simetria em relacao ao eixo `y`, cada modo deve usar:

- apenas harmonicos impares, ou
- apenas harmonicos pares.

No codigo, isso aparece como `odd` e `even`.

## 8. Pontos De Casamento

### Caso `odd`

$$
\theta_m =
\frac{(m - 1/2)\pi}{2N},
\qquad
m = 1,\ldots,N.
$$

### Caso `even`, com `a/b = 1`

Para componentes de simetria par em torno de `\theta = 0`,

$$
\theta_m =
\frac{(m - 1/2)\pi}{2N},
\qquad
m = 1,\ldots,N.
$$

Para componentes de simetria impar em torno de `\theta = 0`,

$$
\theta_m =
\frac{(m - N - 1/2)\pi}{2(N-1)},
\qquad
m = N+1,\ldots,2N-1.
$$

### Caso `even`, com `a/b \neq 1`

O artigo diz para usar a primeira formula para todos os pontos, exceto pelos dois pontos extremos do componente `z` impar, que devem ser omitidos.

## 9. Uso Pratico No Repositorio

Estas definicoes entram diretamente em:

- escolha da classe modal em `src/core/layout.cpp` e `include/goell/layout.hpp`;
- geracao dos pontos de contorno;
- montagem dos blocos da matriz `Q`.

Se houver duvida na implementacao, este arquivo deve ser lido em conjunto com [02_matriz_global_e_normalizacao.md](02_matriz_global_e_normalizacao.md).

_Continua em: [02_matriz_global_e_normalizacao.md](02_matriz_global_e_normalizacao.md) | [Indice de docs](../README.md)_
