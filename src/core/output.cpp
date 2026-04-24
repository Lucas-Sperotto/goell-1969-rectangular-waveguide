/*
 * output.cpp — Formatação e escrita dos resultados em CSV.
 *
 * Contexto:
 *   Este módulo conecta o pipeline de busca de raízes (solver.cpp) à saída
 *   padrão.  Cada função escreve um CSV com cabeçalho, destinado a ser
 *   consumido por scripts Python (scripts/plot_curves.py, validate_bessel.py).
 *
 *   Fluxo principal (write_root_csv):
 *     Para B ∈ [B_min, B_max] com NB+1 passos:
 *       1. select_candidates_for_B: localiza P' candidatos via "sign" ou "minima"
 *       2. Refinamento parabólico (somente "minima")
 *       3. Cálculo opcional do NullInfo (--null-vector)
 *       4. Escrita de uma linha CSV por modo
 *
 *   Cada linha representa um ponto (B, P') na curva de propagação de um modo
 *   [§3.3, Figs. 5-7].  O campo branch_id distingue os modos dentro de um B.
 *
 * Referência: docs/03.3_curvas_de_propagacao.md  docs/simbolos.md §"CSV".
 */

#include "goell/output.hpp"

#include "goell/bessel.hpp"
#include "goell/diagnostics.hpp"
#include "goell/field.hpp"
#include "goell/matrix.hpp"
#include "goell/solver.hpp"

#include <algorithm>
#include <cmath>
#include <ostream>
#include <utility>
#include <vector>

namespace
{

// Seleciona candidatos a modos para um dado B usando a estratégia configurada.
//
//   "sign"   → varredura DetSample + bissecção (sign_change_roots)
//   "minima" → varredura Sample + mínimos locais (local_minima)
//
// Com P.all_minima == false, retém apenas o candidato de menor merit (modo
// mais próximo da singularidade); usado para rastrear um único ramo.
std::vector<Sample> select_candidates_for_B(const Params &P, double B)
{
    if (P.metric == "det" && P.det_search == "sign")
    {
        // Estratégia por mudança de sinal: mais robusta para modos bem separados.
        const auto det_scan = scan_P_det(P, B);
        return sign_change_roots(P, B, det_scan);
    }

    // Estratégia por mínimos: útil para depuração visual do perfil de merit(P').
    auto coarse     = scan_P(P, B);
    auto candidates = local_minima(coarse);

    if (P.allow_edge_minima)
    {
        auto edge = edge_minima(coarse);
        candidates.insert(candidates.end(), edge.begin(), edge.end());
    }

    // Ordena por P' crescente para saída determinística
    std::sort(candidates.begin(), candidates.end(), [](const Sample &a, const Sample &b)
              { return a.Pprime < b.Pprime; });

    if (!P.all_minima && !candidates.empty())
    {
        // Retém apenas o modo de menor merit (mais próximo de det(Q)=0)
        candidates = {*std::min_element(candidates.begin(), candidates.end(),
                                        [](const Sample &a, const Sample &b)
                                        { return a.merit < b.merit; })};
    }

    return candidates;
}

} // namespace

// Escreve tabela de validação das funções de Bessel: 13 pares (n, x) cobrem
// zeros de J₀, J₁, j₂ e regiões de interesse para verificar a implementação.
// Comparada com tabelas numéricas por validate_bessel.py (70/70 testes PASS).
void write_bessel_csv(std::ostream &out)
{
    out << "n,x,Jn,Jn_prime,Kn,Kn_prime\n";
    const std::vector<std::pair<int, double>> pts = {
        {0, 0.5}, {0, 1.0}, {0, 2.0}, {0, 2.4048255577}, // 2.4048 ≈ 1º zero de J₀
        {1, 0.5}, {1, 1.0}, {1, 2.0}, {1, 3.8317059702}, // 3.8317 ≈ 1º zero de J₁
        {2, 0.5}, {2, 1.0}, {2, 2.0},
        {3, 1.0}, {3, 3.0},
    };

    for (const auto &[n, x] : pts)
    {
        out << n << "," << x << ","
            << Jn(n, x) << "," << Jn_prime(n, x) << ","
            << Kn(n, x) << "," << Kn_prime(n, x) << "\n";
    }
}

// Escreve varredura bruta de merit(P') para um B fixo (P.dump_B).
// Permite visualizar o perfil de ln|det(Q)| vs P' e identificar manualmente
// a posição dos modos antes de ajustar os parâmetros de busca.
void write_dump_scan_csv(std::ostream &out, const Params &P)
{
    if (P.dump_det_sign && P.metric == "det")
        out << "B,Pprime,merit,det_sign,parity,phase,geometry\n";
    else
        out << "B,Pprime,merit,parity,phase,geometry\n";

    for (const auto &sample : scan_P(P, P.dump_B))
    {
        if (P.dump_det_sign && P.metric == "det")
        {
            // Inclui sinal de det(Q) para visualizar onde ocorrem mudanças de sinal.
            const auto info = determinant_info(assemble_Q(P, sample.B, sample.Pprime));
            out << sample.B << "," << sample.Pprime << "," << sample.merit << ","
                << info.sign << "," << parity_name(P.parity) << "," << phase_name(P.phase) << ","
                << geometry_name(P.geometry_mode) << "\n";
        }
        else
        {
            out << sample.B << "," << sample.Pprime << "," << sample.merit << ","
                << parity_name(P.parity) << "," << phase_name(P.phase) << ","
                << geometry_name(P.geometry_mode) << "\n";
        }
    }
}

// Saída principal: curvas de propagação P'(B) para todos os modos encontrados.
//
// Para cada B no intervalo discreto [B_min, B_max]:
//   - Localiza candidatos a modo com select_candidates_for_B
//   - Refina P' com interpolação parabólica (somente modo "minima")
//   - Escreve uma linha CSV por modo, com branch_id=k (k=0,1,2,…)
//
// Se --null-vector: também computa e escreve Ez_frac, Hz_frac, mode_class
// para classificar o modo em EH / HE / hybrid [§2.4, §2.5].
void write_root_csv(std::ostream &out, const Params &P)
{
    // Cabeçalho do CSV
    if (P.null_vector)
        out << "branch_id,B,Pprime,merit,parity,phase,geometry,Ez_frac,Hz_frac,mode_class\n";
    else
        out << "branch_id,B,Pprime,merit,parity,phase,geometry\n";

    // Iteração sobre B: NB+1 valores linearmente espaçados em [B_min, B_max].
    // NB == 0 → apenas B = B_min (útil para inspecionar um único ponto de frequência).
    for (int iB = 0; iB <= P.NB; ++iB)
    {
        const double B = (P.NB <= 0)
            ? P.B_min
            : P.B_min + (P.B_max - P.B_min) * static_cast<double>(iB) / static_cast<double>(P.NB);

        if (std::fabs(B) < 1e-12)
            continue; // B = 0 não faz sentido físico (λ₀ → ∞)

        const auto candidates = select_candidates_for_B(P, B);

        for (std::size_t k = 0; k < candidates.size(); ++k)
        {
            double pref  = candidates[k].Pprime;
            double merit = candidates[k].merit;

            // Refinamento parabólico para o método "minima" (não necessário em "sign",
            // pois a bissecção já convergiu para a raiz exata).
            if (!(P.metric == "det" && P.det_search == "sign"))
            {
                pref  = refine_local_minimum(P, B, pref);
                merit = merit_value(P, B, pref);
            }

            // Colunas obrigatórias: branch_id, B, P', merit, família de modo.
            out << k << "," << B << "," << pref << "," << merit << ","
                << parity_name(P.parity) << "," << phase_name(P.phase) << ","
                << geometry_name(P.geometry_mode);

            // Colunas opcionais de classificação modal [F3.1 / §2.4-2.5].
            if (P.null_vector)
            {
                const NullInfo info = compute_null_info(P, B, pref);
                out << "," << info.ez_frac << "," << info.hz_frac << "," << info.mode_class;
            }

            out << "\n";
        }
    }
}

// Escreve o mapa de campo 2D para o modo (P.field_B, P.field_Pprime).
// Chama compute_field_grid (todos os cálculos em C++) e serializa o resultado.
// O Python apenas lê este CSV e plota — nenhum cálculo de campo no Python.
void write_field_map_csv(std::ostream &out, const Params &P)
{
    out << "x,y,inside,Ez,Hz,Er,Etheta,Hr,Htheta,Ex,Ey,Hx,Hy\n";

    const auto grid = compute_field_grid(P, P.field_B, P.field_Pprime);

    for (const auto &fp : grid)
    {
        out << fp.x      << "," << fp.y      << ","
            << (fp.inside ? 1 : 0)           << ","
            << fp.Ez     << "," << fp.Hz     << ","
            << fp.Er     << "," << fp.Etheta << ","
            << fp.Hr     << "," << fp.Htheta << ","
            << fp.Ex     << "," << fp.Ey     << ","
            << fp.Hx     << "," << fp.Hy     << "\n";
    }
}
