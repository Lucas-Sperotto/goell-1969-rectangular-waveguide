# 2. Derivação das Equações - Parte 1

_Navegacao: [README geral](../README.md) | [Indice de docs](README.md) | [Anterior: 1. Introducao](01_introducao.md) | [Proximo: 2.1 Efeitos da simetria](02.1_efeitos_da_simetria.md)_

O guia de onda considerado aqui consiste em um núcleo retangular de constante dielétrica $\varepsilon_1$, cercado por um meio infinito de constante dielétrica $\varepsilon_0$. Ambos os meios são assumidos isotrópicos e com permeabilidade igual à do espaço livre, $\mu_0$. A Figura 2 mostra os sistemas de coordenadas utilizados neste trabalho, retangulares e cilíndricos, bem como as dimensões da barra. A direção de propagação é na direção $+z$ (em direção ao observador).

![Figura 2 — Dimensões e sistemas de coordenadas.](img/fig_02.png)

Figura 2 — Dimensões e sistemas de coordenadas.

Em coordenadas cilíndricas, as soluções de campo das equações de Maxwell assumem a forma de funções de Bessel e funções de Bessel modificadas multiplicadas por funções trigonométricas e por suas derivadas. Para que a propagação ocorra na direção $z$, as soluções de campo devem ser dadas por funções de Bessel no núcleo e por funções de Bessel modificadas no exterior. Como as funções de Bessel de segunda espécie possuem um polo na origem, e as funções de Bessel modificadas de primeira espécie possuem um polo no infinito, assume-se que a variação radial dos campos seja dada por uma soma de funções de Bessel de primeira espécie e de suas derivadas no interior do núcleo, e por uma soma de funções de Bessel modificadas e de suas derivadas no exterior do núcleo.

Em coordenadas cilíndricas, as componentes em $z$ dos campos elétrico e magnético são dadas por:

$E_{zi} = \sum_{n=0}^{\infty} a_n J_n(hr)\,\sin(n\theta + \phi_n)\,\exp\left[i(k_z z - \omega t)\right]$

**Equação (1a).**

$H_{zi} = \sum_{n=0}^{\infty} b_n J_n(hr)\,\sin(n\theta + \psi_n)\,\exp\left[i(k_z z - \omega t)\right]$

**Equação (1b).**

no interior do núcleo, e por:

$E_{zo} = \sum_{n=0}^{\infty} c_n K_n(pr)\,\sin(n\theta + \phi_n)\,\exp\left[i(k_z z - \omega t)\right]$

**Equação (1c).**

$H_{zo} = \sum_{n=0}^{\infty} d_n K_n(pr)\,\sin(n\theta + \psi_n)\,\exp\left[i(k_z z - \omega t)\right]$

**Equação (1d).**

no exterior do núcleo, onde $\omega$ é a frequência angular e $k_z$ é a constante de propagação longitudinal. As constantes de propagação transversais são dadas por:

$h = \left(k_1^2 - k_z^2\right)^{1/2}$

**Equação (2a).**

$p = \left(k_z^2 - k_0^2\right)^{1/2}$

**Equação (2b).**

onde $k_1 = \omega(\mu_0\varepsilon_1)^{1/2}$ e $k_0 = \omega(\mu_0\varepsilon_0)^{1/2}$. Os termos $J_n$ e $K_n$ são, respectivamente, as funções de Bessel de ordem $n$ e as funções de Bessel modificadas de ordem $n$, enquanto $\phi_n$ e $\psi_n$ são ângulos de fase arbitrários.

As componentes transversais dos campos são dadas por:

$E_r = \dfrac{i k_z}{k^2 - k_z^2}\left[\dfrac{\partial E_z}{\partial r} + \left(\dfrac{\mu_0\omega}{k_z r}\right)\dfrac{\partial H_z}{\partial \theta}\right]$

**Equação (3a).**

$E_\theta = \dfrac{i k_z}{k^2 - k_z^2}\left[\dfrac{1}{r}\dfrac{\partial E_z}{\partial \theta} - \left(\dfrac{\mu_0\omega}{k_z}\right)\dfrac{\partial H_z}{\partial r}\right]$

**Equação (3b).**

$H_r = \dfrac{i k_z}{k^2 - k_z^2}\left[-\left(\dfrac{k^2}{\mu_0\omega k_z r}\right)\dfrac{\partial E_z}{\partial \theta} + \dfrac{\partial H_z}{\partial r}\right]$

**Equação (3c).**

$H_\theta = \dfrac{i k_z}{k^2 - k_z^2}\left[\left(\dfrac{k^2}{\mu_0\omega k_z}\right)\dfrac{\partial E_z}{\partial r} + \dfrac{1}{r}\dfrac{\partial H_z}{\partial \theta}\right]$

**Equação (3d).**

onde $k$ pode ser tanto $k_1$ quanto $k_0$.

Por fim, a componente do campo elétrico tangente ao núcleo retangular é dada por:

$E_t = \pm\left(E_r\sin\theta + E_\theta\cos\theta\right), \qquad -\theta_c < \theta < \theta_c, \qquad \pi - \theta_c < \theta < \pi + \theta_c$

**Equação (4a).**

ou

$E_t = \pm\left(-E_r\cos\theta + E_\theta\sin\theta\right), \qquad \theta_c < \theta < \pi - \theta_c, \qquad \pi + \theta_c < \theta < 2\pi - \theta_c$

**Equação (4b).**

onde $\theta_c$ é o ângulo que uma reta radial traçada até o canto no primeiro quadrante forma com o eixo $x$. Expressões análogas existem para o campo magnético tangencial.

_Continua em: [2.1 Efeitos da simetria](02.1_efeitos_da_simetria.md) | [Indice de docs](README.md)_
