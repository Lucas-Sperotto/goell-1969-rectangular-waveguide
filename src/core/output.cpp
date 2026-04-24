#include "goell/output.hpp"

#include "goell/bessel.hpp"
#include "goell/diagnostics.hpp"
#include "goell/matrix.hpp"
#include "goell/solver.hpp"

#include <algorithm>
#include <cmath>
#include <ostream>
#include <utility>
#include <vector>

namespace
{
std::vector<Sample> select_candidates_for_B(const Params &P, double B)
{
    if (P.metric == "det" && P.det_search == "sign")
    {
        const auto det_scan = scan_P_det(P, B);
        return sign_change_roots(P, B, det_scan);
    }

    auto coarse = scan_P(P, B);
    auto candidates = local_minima(coarse);
    if (P.allow_edge_minima)
    {
        auto edge = edge_minima(coarse);
        candidates.insert(candidates.end(), edge.begin(), edge.end());
    }

    std::sort(candidates.begin(), candidates.end(), [](const Sample &a, const Sample &b)
              { return a.Pprime < b.Pprime; });

    if (!P.all_minima && !candidates.empty())
    {
        candidates = {*std::min_element(candidates.begin(), candidates.end(), [](const Sample &a, const Sample &b)
                                        { return a.merit < b.merit; })};
    }

    return candidates;
}
} // namespace

void write_bessel_csv(std::ostream &out)
{
    out << "n,x,Jn,Jn_prime,Kn,Kn_prime\n";
    const std::vector<std::pair<int, double>> pts = {
        {0, 0.5}, {0, 1.0}, {0, 2.0}, {0, 2.4048255577},
        {1, 0.5}, {1, 1.0}, {1, 2.0}, {1, 3.8317059702},
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

void write_root_csv(std::ostream &out, const Params &P)
{
    if (P.null_vector)
        out << "branch_id,B,Pprime,merit,parity,phase,geometry,Ez_frac,Hz_frac,mode_class\n";
    else
        out << "branch_id,B,Pprime,merit,parity,phase,geometry\n";

    for (int iB = 0; iB <= P.NB; ++iB)
    {
        const double B = (P.NB <= 0)
                             ? P.B_min
                             : P.B_min + (P.B_max - P.B_min) * static_cast<double>(iB) / static_cast<double>(P.NB);

        if (std::fabs(B) < 1e-12)
            continue;

        const auto candidates = select_candidates_for_B(P, B);
        for (std::size_t k = 0; k < candidates.size(); ++k)
        {
            double pref = candidates[k].Pprime;
            double merit = candidates[k].merit;

            if (!(P.metric == "det" && P.det_search == "sign"))
            {
                pref = refine_local_minimum(P, B, pref);
                merit = merit_value(P, B, pref);
            }

            out << k << "," << B << "," << pref << "," << merit << ","
                << parity_name(P.parity) << "," << phase_name(P.phase) << ","
                << geometry_name(P.geometry_mode);

            if (P.null_vector)
            {
                const NullInfo info = compute_null_info(P, B, pref);
                out << "," << info.ez_frac << "," << info.hz_frac << "," << info.mode_class;
            }

            out << "\n";
        }
    }
}
