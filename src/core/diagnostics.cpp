/*
 * diagnostics.cpp — Determinante, SVD e classificação modal da matriz Q.
 *
 * Contexto físico (artigo §2.7, §3.1):
 *   Para cada par (B, P'), a condição modal é det(Q) = 0  [Eq. 8].
 *   Como Q pode ter dimensão 4N × 4N com N = 5…10, det(Q) pode exceder a
 *   faixa double, portanto usa-se ln|det(Q)| = Σ ln|diagonal da LU|.
 *
 *   Duas métricas alternativas para localizar os zeros:
 *
 *   1. "det" (padrão): ln|det(Q)| via LU com pivotamento total (FullPivLU).
 *      Identificar raízes por mudança de sinal de det(Q) é mais robusto que
 *      buscar mínimos de ln|det(Q)| (que pode ter mínimos espúrios).
 *
 *   2. "sv": log₁₀(σ_min/σ_max) via SVD de Jacobi.  Em modos guiados,
 *      σ_min → 0, então a métrica → −∞.  Mais caro, mas insensível a
 *      pivotamento e útil para diagnóstico de condicionamento.
 *
 *   O vetor nulo (menor vetor singular de Q) contém os coeficientes modais
 *   e é usado para classificar o modo em EH, HE ou híbrido (§2.4, §2.5).
 *
 * Referência: docs/02.7_metodo_de_computacao.md  docs/03.1_precisao.md
 *             docs/simbolos.md §"Critério modal" e §"Vetor nulo".
 */

#include "goell/diagnostics.hpp"

#include "goell/layout.hpp"
#include "goell/matrix.hpp"

#include <Eigen/Dense>

#include <cmath>

using Eigen::ComputeThinU;
using Eigen::ComputeThinV;
using Eigen::FullPivLU;
using Eigen::JacobiSVD;
using Eigen::MatrixXd;
using Eigen::VectorXd;

// Calcula ln|det(Q)| somando os logaritmos dos elementos diagonais da fatoração LU.
// Retorna −∞ se qualquer diagonal é zero ou não-finita (Q singular ou overflow).
// FullPivLU usa pivotamento completo (linhas e colunas) para máxima estabilidade.
double logabs_det(const MatrixXd &Q)
{
    FullPivLU<MatrixXd> lu(Q);
    const auto &LU = lu.matrixLU();
    double sum = 0.0;

    for (int i = 0; i < LU.rows(); ++i)
    {
        const double diagonal = std::fabs(LU(i, i));
        if (!(diagonal > 0.0) || !std::isfinite(diagonal))
            return -std::numeric_limits<double>::infinity();
        sum += std::log(diagonal); // ln|det| = Σ ln|Uᵢᵢ|  [LU: det(A)=det(L)·det(U)]
    }

    return sum;
}

// Calcula DeterminantInfo: ln|det(Q)| e sinal de det(Q).
// O sinal é obtido combinando o sinal das permutações P e Q (FullPivLU) com
// os sinais dos elementos diagonais da fatoração LU: det = det(P)·det(L)·det(U)·det(Q_perm).
// Retorna sign = 0 se Q é singular ou se ocorre overflow numérico.
DeterminantInfo determinant_info(const MatrixXd &Q)
{
    FullPivLU<MatrixXd> lu(Q);
    const auto &LU = lu.matrixLU();

    DeterminantInfo info;
    info.logabs = 0.0;

    // Sinal das permutações (cada troca de linha/coluna inverte o sinal do det).
    int sign = static_cast<int>(
        std::round(lu.permutationP().determinant() * lu.permutationQ().determinant()));
    if (sign == 0)
        sign = 1;

    for (int i = 0; i < LU.rows(); ++i)
    {
        const double diagonal = LU(i, i);
        if (!std::isfinite(diagonal) || std::fabs(diagonal) <= 0.0)
        {
            // Diagonal nula ou infinita indica singularidade: det(Q) = 0.
            info.logabs = -std::numeric_limits<double>::infinity();
            info.sign   = 0;
            return info;
        }

        info.logabs += std::log(std::fabs(diagonal));
        sign *= (diagonal < 0.0) ? -1 : 1; // cada diagonal negativa inverte o sinal
    }

    info.sign = sign;
    return info;
}

// Calcula log₁₀(σ_min / σ_max) como métrica de proximidade à singularidade.
// Usa JacobiSVD (mais precisa que BDCSVD para matrizes pequenas quadradas).
// Valores singulares em ordem decrescente: singular_values(0) = σ_max,
// singular_values.tail(1)(0) = σ_min.
// Em um modo guiado: σ_min → 0  →  log₁₀(σ_min/σ_max) → −∞.
double log10_sigma_rel(const MatrixXd &Q)
{
    JacobiSVD<MatrixXd> svd(Q, ComputeThinU | ComputeThinV);
    const auto singular_values = svd.singularValues();
    if (singular_values.size() == 0 || singular_values(0) <= 0.0)
        return 0.0;
    return std::log10(singular_values.tail(1)(0) / singular_values(0));
}

// Calcula o NullInfo para o ponto (B, P') — vetor nulo e classificação modal.
//
// O vetor singular direito v que corresponde ao menor σ de Q satisfaz Q·v ≈ 0
// quando det(Q) ≈ 0.  Esse v contém os coeficientes modais normalizados
// [aₙ, bₙ, cₙ, dₙ]ᵀ segundo o leiaute [A|B|C|D] de ColumnLayout.
//
// A fração de energia eletromagnética em Ez e Hz é calculada como:
//   ez_energy = ‖v[A]‖² + ‖v[C]‖²   (coeficientes dos blocos de Ez)
//   hz_energy = ‖v[B]‖² + ‖v[D]‖²   (coeficientes dos blocos de Hz)
//   ez_frac   = ez_energy / (ez_energy + hz_energy)
//
// Classificação (§2.4):
//   ez_frac > 0.7 → EH    (modo eletrico: Ez domina, análogo ao TM circular)
//   hz_frac > 0.7 → HE    (modo magnético: Hz domina, análogo ao TE circular)
//   caso contrário → hybrid
NullInfo compute_null_info(const Params &P, double B, double Pprime)
{
    const ColumnLayout layout = build_layout(P);
    const MatrixXd Q = assemble_Q(P, B, Pprime);

    NullInfo info;
    if (Q.cols() == 0)
        return info;

    // ComputeThinV: apenas os N vetores singulares à direita; suficiente para obter v.
    JacobiSVD<MatrixXd> svd(Q, ComputeThinV);
    if (svd.matrixV().cols() == 0)
        return info;

    // Último vetor singular (índice ncols−1) → menor σ → vetor nulo aproximado.
    const VectorXd v = svd.matrixV().col(svd.matrixV().cols() - 1);

    const int nA = static_cast<int>(layout.ez_orders.size()); // tamanho dos blocos A e C
    const int nB = static_cast<int>(layout.hz_orders.size()); // tamanho dos blocos B e D

    // Soma ‖v[A]‖² + ‖v[C]‖² → energia nos coeficientes de Ez
    double ez_energy = 0.0;
    for (int j = layout.offset_A; j < layout.offset_A + nA; ++j)
        ez_energy += v(j) * v(j);
    for (int j = layout.offset_C; j < layout.offset_C + nA; ++j)
        ez_energy += v(j) * v(j);

    // Soma ‖v[B]‖² + ‖v[D]‖² → energia nos coeficientes de Hz
    double hz_energy = 0.0;
    for (int j = layout.offset_B; j < layout.offset_B + nB; ++j)
        hz_energy += v(j) * v(j);
    for (int j = layout.offset_D; j < layout.offset_D + nB; ++j)
        hz_energy += v(j) * v(j);

    const double total = ez_energy + hz_energy;
    if (total <= 0.0)
        return info; // vetor nulo nulo: retorna valores default (hybrid, 0.5/0.5)

    info.ez_frac = ez_energy / total;
    info.hz_frac = hz_energy / total;

    // Classificação modal por limiar de 70% (§2.4 / §2.5)
    if (info.ez_frac > 0.7)
        info.mode_class = "EH";
    else if (info.hz_frac > 0.7)
        info.mode_class = "HE";
    else
        info.mode_class = "hybrid";

    return info;
}

// Retorna o vetor singular direito de menor σ de Q — o "vetor nulo" da matriz.
// Reutiliza a mesma decomposição JacobiSVD de compute_null_info, mas expõe o
// vetor completo para que field.cpp possa avaliar os campos em pontos arbitrários.
// v = [aₙ | bₙ | cₙ | dₙ]ᵀ com leiaute conforme ColumnLayout.
VectorXd compute_null_vector(const Params &P, double B, double Pprime)
{
    const MatrixXd Q = assemble_Q(P, B, Pprime);
    JacobiSVD<MatrixXd> svd(Q, ComputeThinV);
    // Última coluna de matrixV() = vetor singular de menor σ (SVD em ordem decrescente).
    return svd.matrixV().col(svd.matrixV().cols() - 1);
}
