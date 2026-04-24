# Lista de Símbolos

_Navegação: [README geral](../README.md) | [Índice de docs](README.md)_

Este documento reúne todos os símbolos usados no projeto, organizados por categoria. Para cada símbolo são indicados: o nome no artigo de Goell (1969), o nome adotado nos docs e o identificador correspondente no código C++ modular (`include/goell/` e `src/core/`) ou no CSV de saída.

---

## 1. Geometria do Guia

| Símbolo | Definição | Artigo | Docs | Código |
| --------- | ----------- | -------- | ------ | -------- |
| $a$ | Dimensão completa do núcleo na direção $x$ | $a$ | `a` | `P.a_over_b * b` |
| $b$ | Dimensão completa do núcleo na direção $y$ (dimensão de referência) | $b$ | `b` | unidade de normalização |
| $a/b$ | Razão de aspecto | — | `a/b` | `P.a_over_b` |
| $\theta_c$ | Ângulo do canto: $\theta_c = \arctan(b/a)$ | $\theta_c$ | `theta_c` | `atan(1.0 / P.a_over_b)` |
| $r$ | Coordenada radial cilíndrica | $r$ | `r` | `bp.r` (normalizado por $b$) |
| $\theta$ | Coordenada angular cilíndrica | $\theta$ | `theta` | `bp.theta` |
| $r_m$ | Distância radial ao ponto de casamento $m$ na fronteira | $r_m$ | `r_m` | `bp.r` |
| $\theta_m$ | Ângulo do $m$-ésimo ponto de casamento | $\theta_m$ | `theta_m` | `theta` no laço de pontos |
| $R$ | Projeção radial da componente tangencial (eqs. 4, 7) | $R$ | `R` | `bp.R` |
| $T$ | Projeção azimutal da componente tangencial (eqs. 4, 7) | $T$ | `T` | `bp.T` |

### Definição de $R$ e $T$ por trecho da fronteira

| Trecho | Condição | $R$ | $T$ | $r_m$ |
| -------- | ---------- | ----- | ----- | -------- |
| Lado vertical ($x = a/2$) | $\theta_m < \theta_c$ | $\sin\theta_m$ | $\cos\theta_m$ | $(a/2)/\cos\theta_m$ |
| Lado horizontal ($y = b/2$) | $\theta_m > \theta_c$ | $-\cos\theta_m$ | $\sin\theta_m$ | $(b/2)/\sin\theta_m$ |

---

## 2. Parâmetros Eletromagnéticos

| Símbolo | Definição | Artigo | Docs | Código |
| --------- | ----------- | -------- | ------ | -------- |
| $\varepsilon_1$ | Permissividade do núcleo | $\varepsilon_1$ | `eps_1` | `P.n_r * P.n_r * eps_0` |
| $\varepsilon_0$ | Permissividade do exterior (vácuo/ar) | $\varepsilon_0$ | `eps_0` | referência |
| $\varepsilon_r = \varepsilon_1/\varepsilon_0$ | Permissividade relativa | $\varepsilon_r$ | `eps_r` | `eps_r = P.n_r * P.n_r` |
| $\mu_0$ | Permeabilidade (igual nos dois meios) | $\mu_0$ | `mu_0` | absorvida em $Z_0$ |
| $\omega$ | Frequência angular | $\omega$ | `omega` | absorvida em $k_0$, $k_1$ |
| $Z_0 = \sqrt{\mu_0/\varepsilon_0}$ | Impedância do espaço livre | $Z_0$ | `Z_0` | absorvida por normalização (ver Sec. 2.7) |

---

## 3. Números de Onda e Propagação

| Símbolo | Definição | Artigo | Docs | Código |
| --------- | ----------- | -------- | ------ | -------- |
| $k_0 = \omega\sqrt{\mu_0\varepsilon_0}$ | Número de onda no exterior | $k_0$ | `k_0` | variável de normalização |
| $k_1 = \omega\sqrt{\mu_0\varepsilon_1}$ | Número de onda no núcleo | $k_1$ | `k_1` | $= n_r k_0$ |
| $n_r = k_1/k_0$ | Índice de refração relativo, eq. (13) | $n_r$ | `n_r` | `P.n_r` |
| $k_z$ | Constante de propagação longitudinal | $k_z$ | `k_z` | calculado via $\mathcal{P}^2$ |
| $k_z/k_0$ | Constante de propagação normalizada | — | `kz_over_k0` | `kz_over_k0 = sqrt(1 + (eps_r-1)*Pprime)` |
| $h = \sqrt{k_1^2 - k_z^2}$ | Número de onda transversal interior, eq. (2a) | $h$ | `h` | `h_scaled / b` |
| $p = \sqrt{k_z^2 - k_0^2}$ | Número de onda transversal exterior, eq. (2b) | $p$ | `p` | `p_scaled / b` |
| $h_\text{scaled} = \pi\mathcal{B}\sqrt{1-\mathcal{P}^2}$ | $h$ normalizado por $b$ (argumento das Bessel internas) | — | `h_scaled` | `h_scaled = PI * B * sqrt(max(0, 1-Pprime))` |
| $p_\text{scaled} = \pi\mathcal{B}\sqrt{\mathcal{P}^2}$ | $p$ normalizado por $b$ (argumento das Bessel externas) | — | `p_scaled` | `p_scaled = PI * B * sqrt(max(0, Pprime))` |

---

## 4. Variáveis Normalizadas (curvas de propagação)

| Símbolo | Definição | Eq. | Artigo | Código (CSV) |
| --------- | ----------- | ----- | -------- | -------------- |
| $\mathcal{P}^2 = \dfrac{(k_z/k_0)^2-1}{n_r^2-1}$ | Constante de propagação normalizada; eixo vertical dos gráficos | (11) | $\rho^2$ / $P^2$ | `Pprime` |
| $\mathcal{B} = \dfrac{2b}{\lambda_0}\sqrt{n_r^2-1}$ | Frequência normalizada; eixo horizontal dos gráficos | (16) | $\mathcal{B}$ | `B` |
| $\lambda_0 = 2\pi/k_0$ | Comprimento de onda no exterior | — | $\lambda_0$ | — |
| $\mathcal{R} = rk_0\sqrt{n_r^2-1}$ | Coordenada radial normalizada, eq. (12) | (12) | $\mathcal{R}$ | — |

**Relações diretas:**

$$
hr = \mathcal{R}\sqrt{1 - \mathcal{P}^2}, \qquad pr = \mathcal{R}\,\mathcal{P}
$$

Em termos das variáveis do código (com $r$ normalizado por $b$):

$$
h_\text{scaled} \cdot \hat{r} = \pi\mathcal{B}\sqrt{1-\mathcal{P}^2}\cdot\hat{r}, \qquad
p_\text{scaled} \cdot \hat{r} = \pi\mathcal{B}\sqrt{\mathcal{P}^2}\cdot\hat{r}
$$

onde $\hat{r} = r/b$.

**Intervalo físico:** $\mathcal{P}^2 \in (0,1)$, $\mathcal{B} > 0$.

---

## 5. Funções Especiais

| Símbolo | Definição | Artigo | Docs | Código |
| --------- | ----------- | -------- | ------ | -------- |
| $J_n(x)$ | Função de Bessel de primeira espécie, ordem $n$ | $J_n$ | `J_n(x)` | `Jn(n, x)` |
| $K_n(x)$ | Função de Bessel modificada de segunda espécie, ordem $n$ | $K_n$ | `K_n(x)` | `Kn(n, x)` |
| $J_n'(x)$ | Derivada de $J_n$ em relação ao argumento | $J_n'$ | `J_n'(x)` | `Jn_prime(n, x)` |
| $K_n'(x)$ | Derivada de $K_n$ em relação ao argumento | $K_n'$ | `K_n'(x)` | `Kn_prime(n, x)` |

**Fórmulas de recorrência utilizadas:**

$$
J_0'(x) = -J_1(x), \qquad J_n'(x) = \tfrac{1}{2}\bigl[J_{n-1}(x) - J_{n+1}(x)\bigr], \quad n \geq 1
$$

$$
K_0'(x) = -K_1(x), \qquad K_n'(x) = -\tfrac{1}{2}\bigl[K_{n-1}(x) + K_{n+1}(x)\bigr], \quad n \geq 1
$$

### Formas compactas usadas nos elementos de matriz (eqs. 7)

| Símbolo | Definição explícita | Artigo | Docs | Código |
| --------- | --------------------- | -------- | ----- | -------- |
| $J$ | $J_n(hr_m)$ | $J$ | `J` | `J0 = Jn(n, hr)` |
| $K$ | $K_n(pr_m)$ | $K$ | `K` | `K0 = Kn(n, pr)` |
| $\widetilde{J}' = J_n'(hr_m)/h$ | Derivada radial de $J$ normalizada por $h$ | $\widetilde{J}'$ | `J_r` | `Jr = Jn_prime(n, hr) / h_scaled` |
| $\widetilde{K}' = K_n'(pr_m)/p$ | Derivada radial de $K$ normalizada por $p$ | $\widetilde{K}'$ | `K_r` | `Kr = Kn_prime(n, pr) / p_scaled` |
| $\widetilde{J} = nJ_n(hr_m)/(h^2 r_m)$ | Contribuição azimutal de $J$ | $\widetilde{J}$ | $J_\theta$ | `Jth = n * J0 / (h_scaled^2 * bp.r)` |
| $\widetilde{K} = nK_n(pr_m)/(p^2 r_m)$ | Contribuição azimutal de $K$ | $\widetilde{K}$ | $K_\theta$ | `Kth = n * K0 / (p_scaled^2 * bp.r)` |

> **Nota de notação:** O artigo usa barras e primas ($\bar{J}$, $\bar{J}'$) de forma ambígua no scan. Os docs e o código adotam os sufixos `_r` (derivada radial) e `_th` (contribuição azimutal) para eliminar a ambiguidade.

---

## 6. Expansões Modais e Coeficientes

| Símbolo | Definição | Artigo | Docs | Código |
| --------- | ----------- | -------- | ------ | -------- |
| $n$ | Ordem harmônica | $n$ | `n` | `n` (inteiro) |
| $N$ | Número de harmônicos da classe escolhida | $N$ | `N` | `P.N` |
| $a_n$ | Coeficiente de $E_z$ no interior (bloco A) | $a_n$ | `a_n` | `v(L.offset_A + i)` no vetor nulo |
| $b_n$ | Coeficiente de $H_z$ no interior (bloco B) | $b_n$ | `b_n` | `v(L.offset_B + i)` |
| $c_n$ | Coeficiente de $E_z$ no exterior (bloco C) | $c_n$ | `c_n` | `v(L.offset_C + i)` |
| $d_n$ | Coeficiente de $H_z$ no exterior (bloco D) | $d_n$ | `d_n` | `v(L.offset_D + i)` |
| $\phi_n$ | Fase da expansão de $E_z$, eq. (1a) | $\phi_n$ | `phi_n` | parâmetro `--phase` |
| $\psi_n$ | Fase da expansão de $H_z$, eq. (1b) | $\psi_n$ | `psi_n` | determinado por `--phase` |
| $\phi = 0$ | Primeira família de fase: $S = \sin(n\theta)$, $C = \cos(n\theta)$ | — | `phi0` | `PhaseFamily::phi0` |
| $\phi = \pi/2$ | Segunda família de fase: $S = \cos(n\theta)$, $C = -\sin(n\theta)$ | — | `phi90` | `PhaseFamily::phi90` |

**Funções angulares de base:**

$$
S = \sin(n\theta_m + \phi), \qquad C = \cos(n\theta_m + \phi)
$$

---

## 7. Paridade dos Harmônicos

| Valor | Definição | Código |
| ------- | ----------- | -------- |
| `odd` | Apenas harmônicos ímpares: $n = 1, 3, 5, \ldots, 2N-1$ | `HarmonicParity::odd` |
| `even` | Apenas harmônicos pares: $n = 0, 2, 4, \ldots, 2(N-1)$ | `HarmonicParity::even` |

O caso `even` gera $4N-2$ equações (em vez de $4N$) porque o harmônico $n=0$ desaparece em um dos blocos longitudinais dependendo da família de fase.

---

## 8. Pontos de Casamento

| Caso | Fórmula | Código |
| ------ | --------- | -------- |
| `odd` (todos os componentes) | $\theta_m = (m - 1/2)\,\pi/(2N)$, $m=1,\ldots,N$ | `odd_case_thetas(N)` |
| `even`, simetria par em $\theta=0$ | $\theta_m = (m - 1/2)\,\pi/(2N)$, $m=1,\ldots,N$ | `even_symmetry_thetas(N)` |
| `even`, simetria ímpar ($a/b=1$) | $\theta_m = (m - 1/2)\,\pi/(2(N-1))$, $m=1,\ldots,N-1$ | `odd_symmetry_thetas(N)` |
| `even`, componente-$z$ ímpar ($a/b\neq1$) | mesmos $N$ pontos, sem o primeiro e o último | `omit_first_last(thetas)` |

---

## 9. Campos Eletromagnéticos

| Símbolo | Definição | Artigo | Docs |
| --------- | ----------- | -------- | ------ |
| $E_{z1}$, $H_{z1}$ | Componentes longitudinais no interior do núcleo, eqs. (1a–1b) | $E_{zi}$, $H_{zi}$ | `Ez_interior`, `Hz_interior` |
| $E_{z0}$, $H_{z0}$ | Componentes longitudinais no exterior, eqs. (1c–1d) | $E_{zo}$, $H_{zo}$ | `Ez_exterior`, `Hz_exterior` |
| $E_r$, $E_\theta$ | Componentes transversais do campo elétrico, eqs. (3a–3b) | $E_r$, $E_\theta$ | campos avaliados em `field_map.py` |
| $H_r$, $H_\theta$ | Componentes transversais do campo magnético, eqs. (3c–3d) | $H_r$, $H_\theta$ | idem |
| $E_t$ | Campo elétrico tangente à fronteira, eqs. (4a–4b) | $E_t$ | `E_t` (linha de casamento) |
| $H_t$ | Campo magnético tangente à fronteira | $H_t$ | `H_t` (linha de casamento) |

---

## 10. Blocos da Matriz de Casamento Q

A equação matricial global é $[Q][T] = 0$, com

$$
Q =
\begin{bmatrix}
E^{LA} & 0 & -E^{LC} & 0 \\
0 & H^{LB} & 0 & -H^{LD} \\
E^{TA} & E^{TB} & -E^{TC} & -E^{TD} \\
H^{TA} & H^{TB} & -H^{TC} & -H^{TD}
\end{bmatrix}
$$

### Blocos longitudinais

| Bloco | Elemento $(m,n)$ | Eq. artigo | Código |
| ------- | ----------------- | ------------ | -------- |
| $E^{LA}$ | $J\,S$ | (7a) | `eLA = J0 * S` |
| $E^{LC}$ | $K\,S$ | (7b) | `eLC = K0 * S` |
| $H^{LB}$ | $J\,C$ | (7c) | `hLB = J0 * C` |
| $H^{LD}$ | $K\,C$ | (7d) | `hLD = K0 * C` |

### Blocos tangenciais — campo elétrico

| Bloco | Elemento $(m,n)$ | Eq. artigo | Código |
| ------- | ----------------- | ------------ | -------- |
| $E^{TA}$ | $-k_z\left(\widetilde{J}'SR + \widetilde{J}CT\right)$ | (7e) | `eTA = -kz_over_k0 * (Jr*S*R + Jth*C*T)` |
| $E^{TB}$ | $k_0 Z_0\left(\widetilde{J}SR + \widetilde{J}'CT\right)$ | (7f) | `eTB = +(Jth*S*R + Jr*C*T)` ¹ |
| $E^{TC}$ | $k_z\left(\widetilde{K}'SR + \widetilde{K}CT\right)$ | (7g) | `eTC = +kz_over_k0 * (Kr*S*R + Kth*C*T)` |
| $E^{TD}$ | $-k_0 Z_0\left(\widetilde{K}SR + \widetilde{K}'CT\right)$ | (7h) | `eTD = -(Kth*S*R + Kr*C*T)` ¹ |

### Blocos tangenciais — campo magnético

| Bloco | Elemento $(m,n)$ | Eq. artigo | Código |
| ------- | ----------------- | ------------ | -------- |
| $H^{TA}$ | $\varepsilon_r k_0\left(\widetilde{J}CR - \widetilde{J}'ST\right)/Z_0$ | (7i) | `hTA = +eps_r * (Jth*C*R - Jr*S*T)` ¹ |
| $H^{TB}$ | $-k_z\left(\widetilde{J}'CR - \widetilde{J}ST\right)$ | (7j) | `hTB = -kz_over_k0 * (Jr*C*R - Jth*S*T)` |
| $H^{TC}$ | $-k_0\left(\widetilde{K}CR - \widetilde{K}'ST\right)/Z_0$ | (7k) | `hTC = -(Kth*C*R - Kr*S*T)` ¹ |
| $H^{TD}$ | $k_z\left(\widetilde{K}'CR - \widetilde{K}ST\right)$ | (7l) | `hTD = +kz_over_k0 * (Kr*C*R - Kth*S*T)` |

> ¹ Os fatores $k_0 Z_0$ e $k_0/Z_0$ são absorvidos pelo reescalonamento de linhas/colunas (Sec. 2.7.2 do artigo), que não desloca os zeros de $\det(Q)$.

---

## 11. Dimensões da Matriz Q

| Paridade | Dimensão | Blocos por componente |
| ---------- | ---------- | ----------------------- |
| `odd` | $4N \times 4N$ | $N$ pontos para cada um dos 4 tipos de linha |
| `even` | $(4N-2) \times (4N-2)$ | $N$ para componentes pares, $N-1$ para ímpares |

---

## 12. Critério Modal e Busca de Raízes

| Símbolo | Definição | Eq. | Código |
| --------- | ----------- | ----- | -------- |
| $\det(Q) = 0$ | Critério de modo guiado | (19) | `FullPivLU` / mudança de sinal |
| $\log\lvert\det(Q)\rvert$ | Métrica de vale para busca por mínimos | — | `logabs_det(Q)` |
| $\sigma_\min/\sigma_\max$ | Métrica alternativa via SVD | — | `log10_sigma_rel(Q)` |
| `Pscan` | Número de amostras de $\mathcal{P}^2$ por valor de $B$ | — | `P.Pscan` |
| `NB` | Número de intervalos de $B$ no varrimento | — | `P.NB` |

---

## 13. Vetor Nulo e Classificação Modal (flag `--null-vector`)

Ao habilitar `--null-vector`, o solver calcula o vetor nulo de $Q$ via SVD de Jacobi (`JacobiSVD`) no ponto raiz e exporta as colunas adicionais abaixo.

| Símbolo | Definição | CSV |
| --------- | ----------- | ----- |
| $\mathbf{v}$ | Vetor singular direito de $Q$ associado ao menor valor singular | — |
| $\|v_A\|^2 + \|v_C\|^2$ | Energia do vetor nulo nos blocos Ez (coeficientes $a_n$, $c_n$) | — |
| $\|v_B\|^2 + \|v_D\|^2$ | Energia do vetor nulo nos blocos Hz (coeficientes $b_n$, $d_n$) | — |
| $f_{Ez}$ | $= (\|v_A\|^2 + \|v_C\|^2)\,/\,\|\mathbf{v}\|^2$ | `Ez_frac` |
| $f_{Hz}$ | $= (\|v_B\|^2 + \|v_D\|^2)\,/\,\|\mathbf{v}\|^2$ | `Hz_frac` |
| `mode_class` | `EH` se $f_{Ez} > 0{,}7$; `HE` se $f_{Hz} > 0{,}7$; senão `hybrid` | `mode_class` |

---

## 14. Designação dos Modos

| Classe | Característica física | Condição |
| -------- | ----------------------- | ---------- |
| EH | Modo com componente $E_z$ dominante | $f_{Ez} > 0{,}7$ |
| HE | Modo com componente $H_z$ dominante | $f_{Hz} > 0{,}7$ |
| hybrid | Modo misto (ambas as componentes significativas) | $f_{Ez} \leq 0{,}7$ e $f_{Hz} \leq 0{,}7$ |
| TE | Caso limite: $E_z \equiv 0$ (harmônicos ímpares, $\phi=0$) | teórico |
| TM | Caso limite: $H_z \equiv 0$ (harmônicos ímpares, $\phi=\pi/2$) | teórico |

A classificação EH/HE no guia dielétrico retangular é análoga à usada em guias de fibra óptica circular, onde EH$_{mn}$ denota modo com campo elétrico predominantemente longitudinal e HE$_{mn}$ denota campo magnético predominantemente longitudinal.

---

## 15. Parâmetros da Linha de Comando

| Flag | Parâmetro | Padrão | Código |
| ------ | ----------- | -------- | -------- |
| `--parity odd\|even` | Paridade dos harmônicos | `odd` | `P.parity` |
| `--phase phi0\|phi90` | Família de fase | `phi0` | `P.phase` |
| `--a_over_b` | Razão $a/b$ | `1.0` | `P.a_over_b` |
| `--nr` | Índice de refração $n_r$ | `1.01` | `P.n_r` |
| `--N` | Número de harmônicos | `5` | `P.N` |
| `--Bmin`, `--Bmax` | Faixa de $\mathcal{B}$ | `0.1`, `4.0` | `P.B_min`, `P.B_max` |
| `--NB` | Número de intervalos em $\mathcal{B}$ | `40` | `P.NB` |
| `--Pscan` | Amostras de $\mathcal{P}^2$ por $B$ | `160` | `P.Pscan` |
| `--metric det\|sv` | Métrica para detecção de raízes | `det` | `P.metric` |
| `--det-search sign\|minima` | Estratégia de busca | `sign` | `P.det_search` |
| `--no-rescale` | Desativa reescalonamento de linhas/colunas | — | `P.rescale_matrix = false` |
| `--null-vector` | Calcula vetor nulo via SVD em cada raiz | — | `P.null_vector` |
| `--test-bessel` | Exporta tabela de $J_n$, $K_n$ e derivadas | — | `P.test_bessel` |
| `--dump-scan B` | Exporta varredura de $\mathcal{P}^2$ para um único $B$ | — | `P.dump_scan`, `P.dump_B` |

---

## 16. Colunas do CSV de Saída Principal

| Coluna | Tipo | Descrição |
| -------- | ------ | ----------- |
| `branch_id` | inteiro | Índice do modo encontrado em cada valor de $B$ (ordenado por $\mathcal{P}^2$ crescente) |
| `B` | float | Frequência normalizada $\mathcal{B}$ |
| `Pprime` | float | Constante de propagação normalizada $\mathcal{P}^2$ |
| `merit` | float | $\log\lvert\det(Q)\rvert$ ou $\log_{10}(\sigma_\min/\sigma_\max)$ na raiz |
| `parity` | string | `odd` ou `even` |
| `phase` | string | `phi0` ou `phi90` |
| `geometry` | string | `intersection` ou `literal` |
| `Ez_frac` | float | $f_{Ez}$ (apenas com `--null-vector`) |
| `Hz_frac` | float | $f_{Hz}$ (apenas com `--null-vector`) |
| `mode_class` | string | `EH`, `HE` ou `hybrid` (apenas com `--null-vector`) |

---

_Navegação: [README geral](../README.md) | [Índice de docs](README.md)_
