/*
 * diagnostics.hpp — Funções de diagnóstico da matriz Q: det, SVD e vetor nulo.
 *
 * A condição de modo guiado é det(Q) = 0  [Eq. 8 / §2.7].  Em vez de calcular
 * det(Q) diretamente (sujeito a overflow/underflow para N grande), usamos o
 * logaritmo natural de |det(Q)| via decomposição LU.
 *
 * Como alternativa, a menor razão de valores singulares log₁₀(σ_min/σ_max)
 * mede quão próximo Q está da singularidade: em um modo guiado, σ_min → 0 e
 * log₁₀(σ_min/σ_max) → −∞.
 *
 * O vetor nulo (right singular vector de menor σ) contém os coeficientes
 * modais [aₙ, bₙ, cₙ, dₙ] e permite classificar o modo como EH, HE ou híbrido
 * pela fração de energia eletromagnética relativa (§2.4, §2.5).
 *
 * Referência: docs/02.7_metodo_de_computacao.md  docs/03.1_precisao.md
 *             docs/simbolos.md §"Vetor nulo e classificação modal".
 */

#ifndef GOELL_DIAGNOSTICS_HPP
#define GOELL_DIAGNOSTICS_HPP

#include "goell/common.hpp"

#include <Eigen/Dense>
#include <limits>
#include <string>

// ─── Informações do determinante ─────────────────────────────────────────────
//
// Separar log|det| e sinal evita representar det(Q) como double (estouro para N≥6).
// O sinal é usado para detectar raízes por mudança de sinal (det_search = "sign").
struct DeterminantInfo
{
    double logabs = -std::numeric_limits<double>::infinity(); // ln|det(Q)|; −∞ se Q é singular
    int    sign   = 0;  // sinal de det(Q): +1, −1 ou 0 (se Q é singular ou overflow)
};

// ─── Informações do vetor nulo ────────────────────────────────────────────────
//
// Obtidas pela SVD de Q no ponto modal (P', B) que satisfaz det(Q) ≈ 0.
// O vetor singular direito v de menor σ resolve Q·v ≈ 0 e contém
// os coeficientes {aₙ, bₙ, cₙ, dₙ} da expansão modal [Eqs. 1a-d].
//
// A fração de "energia" Ez versus Hz é calculada como a norma quadrada dos
// coeficientes dos blocos A+C (Ez) e B+D (Hz) respectivamente [§2.5]:
//   ez_frac = (‖A‖² + ‖C‖²) / (‖A‖² + ‖B‖² + ‖C‖² + ‖D‖²)
//
// Classificação modal (§2.4):
//   ez_frac > 0.7 → EH    (modo dominado por Ez, como modos TM no guia metálico)
//   hz_frac > 0.7 → HE    (modo dominado por Hz, como modos TE no guia metálico)
//   caso contrário → hybrid (modos completamente híbridos EH/HE)
struct NullInfo
{
    double      ez_frac    = 0.5;     // fração de energia nos coeficientes de Ez ∈ [0,1]
    double      hz_frac    = 0.5;     // fração de energia nos coeficientes de Hz ∈ [0,1]
    std::string mode_class = "hybrid"; // "EH", "HE" ou "hybrid"
};

// Retorna ln|det(Q)| via LU com pivotamento total.  Para uso quando apenas
// a magnitude é necessária (sem informação de sinal).
double logabs_det(const Eigen::MatrixXd &Q);

// Retorna DeterminantInfo com ln|det(Q)| e sinal(det(Q)).
// Usado na busca por raízes via mudança de sinal de det(Q) [§2.7].
DeterminantInfo determinant_info(const Eigen::MatrixXd &Q);

// Retorna log₁₀(σ_min / σ_max) via SVD de Jacobi.
// Mede o quão próxima Q está da singularidade: −∞ em um modo exato,
// 0 quando Q é bem-condicionada (sem modo).  Usado com metric = "sv".
double log10_sigma_rel(const Eigen::MatrixXd &Q);

// Calcula NullInfo para o ponto (B, P') dado.
// Monta Q internamente, aplica SVD e analisa o vetor singular de menor σ.
// Usado quando --null-vector está ativo para classificar o modo [F3.1].
NullInfo compute_null_info(const Params &P, double B, double Pprime);

// Retorna o vetor nulo completo v (right singular vector de menor σ de Q).
// v tem dimensão ncols = 4N (odd) ou 4N-2 (even).
// v = [aₙ | bₙ | cₙ | dₙ]ᵀ com leiaute conforme ColumnLayout [F3.2].
// Usado por compute_field_grid para avaliar os campos em pontos arbitrários.
Eigen::VectorXd compute_null_vector(const Params &P, double B, double Pprime);

#endif // GOELL_DIAGNOSTICS_HPP
