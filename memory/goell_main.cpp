// goell_q_solver.cpp
//
// Reescrita "paper-faithful" do solver CHM de Goell (1969).
//
// A ideia aqui e seguir diretamente as equacoes do artigo:
// - expansoes longitudinais: eqs. (1a)-(1d)
// - campos transversos:      eqs. (3a)-(3d)
// - campo tangencial:        eqs. (4a)-(4b)
// - simetria / fases:        Sec. 2.1
// - pontos de matching:      Sec. 2.2
// - elementos de matriz:     eqs. (6a)-(6d) e (7a)-(7l)
// - matriz global Q:         eq.  (18)
// - criterio modal:          eq.  (19), det(Q)=0
//
// Duas observacoes cruciais do paper, agora explicitadas no codigo:
// 1) "odd/even" no artigo e paridade dos harmônicos n, nao "familia completa".
// 2) alem da paridade, existe a familia de fase da Sec. 2.1:
//      phi = 0     -> S = sin(n*theta),   C = cos(n*theta)
//      phi = pi/2  -> S = cos(n*theta),   C = -sin(n*theta)
//
// Portanto o problema possui quatro classes:
//   (odd harmonics, phi=0), (odd harmonics, phi=pi/2),
//   (even harmonics, phi=0), (even harmonics, phi=pi/2).
//
// Build:
//   g++ -O3 -std=c++17 -I /usr/include/eigen3 src/goell_q_solver.cpp -o build/goell_q_solver
//
// Exemplo:
//   ./build/goell_q_solver --parity odd --phase phi0 --a_over_b 1 --nr 1.01 \
//       --N 5 --Bmin 0.1 --Bmax 4 --NB 40 --Pscan 160 --metric det > out.csv
// Standard library includes
#include <iostream>
#include <vector>
#include <string>
#include <iomanip> // For std::setprecision
#include <algorithm> // For std::min_element, std::sort
#include <cmath>     // For fabs

// Project-specific includes
#include "goell_common.hpp"
#include "goell_bessel.hpp"
#include "goell_matrix.hpp" // For assemble_Q, determinant_info
#include "goell_solver.hpp" // For merit_value, determinant_value, scan_P, scan_P_det, local_minima, edge_minima, sign_change_roots, refine_local_minimum

int main(int argc, char **argv)
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Params P;
    try
    {
        parse_args(argc, argv, P);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[goell_q_solver] " << e.what() << "\n";
        return 1;
    }

    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(15);

    if (P.test_bessel)
    {
        std::cout << "n,x,Jn,Jn_prime,Kn,Kn_prime\n";
        const std::vector<std::pair<int,double>> pts = {
            {0, 0.5}, {0, 1.0}, {0, 2.0}, {0, 2.4048255577},
            {1, 0.5}, {1, 1.0}, {1, 2.0}, {1, 3.8317059702},
            {2, 0.5}, {2, 1.0}, {2, 2.0},
            {3, 1.0}, {3, 3.0},
        };
        for (const auto &[n, x] : pts)
            std::cout << n << "," << x << ","
                 << Jn(n, x) << "," << Jn_prime(n, x) << ","
                 << Kn(n, x) << "," << Kn_prime(n, x) << "\n";
        return 0;
    }

    std::cout.precision(6);

    if (P.dump_scan)
    {
        if (P.dump_det_sign && P.metric == "det")
            std::cout << "B,Pprime,merit,det_sign,parity,phase,geometry\n";
        else
            std::cout << "B,Pprime,merit,parity,phase,geometry\n";

        for (const auto &sample : scan_P(P, P.dump_B))
        {
            if (P.dump_det_sign && P.metric == "det")
            {
                const auto info = determinant_info(assemble_Q(P, sample.B, sample.Pprime));
                std::cout << sample.B << "," << sample.Pprime << "," << sample.merit << ","
                     << info.sign << "," << parity_name(P.parity) << "," << phase_name(P.phase) << ","
                     << geometry_name(P.geometry_mode) << "\n";
            }
            else
            {
                std::cout << sample.B << "," << sample.Pprime << "," << sample.merit << ","
                     << parity_name(P.parity) << "," << phase_name(P.phase) << ","
                     << geometry_name(P.geometry_mode) << "\n";
            }
        }
        return 0;
    }

    std::cout << "branch_id,B,Pprime,merit,parity,phase,geometry\n";

    for (int iB = 0; iB <= P.NB; ++iB)
    {
        const double B = (P.NB <= 0)
                             ? P.B_min
                             : P.B_min + (P.B_max - P.B_min) * static_cast<double>(iB) / static_cast<double>(P.NB);

        if (fabs(B) < 1e-12)
            continue;

        std::vector<Sample> candidates;
        if (P.metric == "det" && P.det_search == "sign")
        {
            const auto det_scan = scan_P_det(P, B);
            candidates = sign_change_roots(P, B, det_scan);
        }
        else
        {
            auto coarse = scan_P(P, B);
            candidates = local_minima(coarse);
            if (P.allow_edge_minima)
            {
                auto e = edge_minima(coarse);
                candidates.insert(candidates.end(), e.begin(), e.end());
            }
            std::sort(candidates.begin(), candidates.end(), [](const Sample &a, const Sample &b)
                 { return a.Pprime < b.Pprime; });

            if (!P.all_minima && !candidates.empty())
            {
                candidates = {*std::min_element(candidates.begin(), candidates.end(), [](const Sample &a, const Sample &b)
                                           { return a.merit < b.merit; })};
            }
        }

        for (size_t k = 0; k < candidates.size(); ++k)
        {
            double pref = candidates[k].Pprime;
            double merit = candidates[k].merit;
            if (!(P.metric == "det" && P.det_search == "sign"))
            {
                pref = refine_local_minimum(P, B, pref);
                merit = merit_value(P, B, pref);
            }
            std::cout << k << "," << B << "," << pref << "," << merit << ","
                 << parity_name(P.parity) << "," << phase_name(P.phase) << ","
                 << geometry_name(P.geometry_mode) << "\n";
        }
    }

    return 0;
}
