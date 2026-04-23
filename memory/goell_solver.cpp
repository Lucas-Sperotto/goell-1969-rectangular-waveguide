#include "goell_solver.hpp"
#include "goell_matrix.hpp" // For assemble_Q, determinant_info, log10_sigma_rel
#include <algorithm> // For std::min, std::max, std::sort, std::min_element
#include <cmath>     // For isfinite, fabs

double merit_value(const Params &P, double B, double Pprime)
{
    const auto Q = assemble_Q(P, B, Pprime);
    return (P.metric == "sv") ? log10_sigma_rel(Q) : determinant_info(Q).logabs;
}

DeterminantInfo determinant_value(const Params &P, double B, double Pprime)
{
    return determinant_info(assemble_Q(P, B, Pprime));
}

std::vector<Sample> scan_P(const Params &P, double B)
{
    std::vector<Sample> out;
    out.reserve(P.Pscan);

    for (int i = 0; i < P.Pscan; ++i)
    {
        const double Pprime = (i + 1.0) / static_cast<double>(P.Pscan + 1);
        out.push_back({B, Pprime, merit_value(P, B, Pprime)});
    }

    return out;
}

std::vector<DetSample> scan_P_det(const Params &P, double B)
{
    std::vector<DetSample> out;
    out.reserve(P.Pscan);

    for (int i = 0; i < P.Pscan; ++i)
    {
        const double Pprime = (i + 1.0) / static_cast<double>(P.Pscan + 1);
        const auto info = determinant_value(P, B, Pprime);
        out.push_back({B, Pprime, info.logabs, info.sign});
    }

    return out;
}

std::vector<Sample> local_minima(const std::vector<Sample> &samples)
{
    std::vector<Sample> mins;
    if (samples.size() < 2)
        return mins;

    for (size_t i = 1; i + 1 < samples.size(); ++i)
    {
        if (std::isfinite(samples[i].merit) &&
            samples[i].merit < samples[i - 1].merit &&
            samples[i].merit < samples[i + 1].merit)
        {
            mins.push_back(samples[i]);
        }
    }

    return mins;
}

std::vector<Sample> edge_minima(const std::vector<Sample> &samples)
{
    std::vector<Sample> mins;
    if (samples.size() < 2)
        return mins;

    if (std::isfinite(samples.front().merit) && samples.front().merit < samples[1].merit)
        mins.push_back(samples.front());

    if (std::isfinite(samples.back().merit) && samples.back().merit < samples[samples.size() - 2].merit)
        mins.push_back(samples.back());

    return mins;
}

std::vector<Sample> sign_change_roots(const Params &P, double B, const std::vector<DetSample> &samples)
{
    std::vector<Sample> roots;
    if (samples.size() < 2)
        return roots;

    auto append_unique = [&](double pref)
    {
        const double clamped = std::min(1.0 - 1e-6, std::max(1e-6, pref));
        if (!roots.empty() && fabs(roots.back().Pprime - clamped) < 1e-5)
            return;
        const auto info = determinant_value(P, B, clamped);
        roots.push_back({B, clamped, info.logabs});
    };

    for (size_t i = 0; i + 1 < samples.size(); ++i)
    {
        const auto &left = samples[i];
        const auto &right = samples[i + 1];

        if (left.sign == 0)
        {
            append_unique(left.Pprime);
            continue;
        }

        if (left.sign != 0 && right.sign != 0 && left.sign != right.sign)
        {
            double a = left.Pprime;
            double b = right.Pprime;
            int sa = left.sign;
            int sb = right.sign;

            for (int it = 0; it < 50; ++it)
            {
                const double mid = 0.5 * (a + b);
                const auto info_mid = determinant_value(P, B, mid);

                if (info_mid.sign == 0) { a = b = mid; break; }
                if (sa != 0 && sa != info_mid.sign) { b = mid; sb = info_mid.sign; }
                else if (sb != 0 && sb != info_mid.sign) { a = mid; sa = info_mid.sign; }
                else { break; }
            }
            append_unique(0.5 * (a + b));
        }
    }
    return roots;
}

double refine_local_minimum(const Params &P, double B, double x0)
{
    auto f = [&](double x) { return merit_value(P, B, std::min(1.0 - 1e-6, std::max(1e-6, x))); };

    const double dx = 2.0 / static_cast<double>(P.Pscan + 1);
    double x1 = std::max(1e-6, x0 - dx);
    double x2 = x0;
    double x3 = std::min(1.0 - 1e-6, x0 + dx);

    for (int it = 0; it < 20; ++it)
    {
        const double y1 = f(x1), y2 = f(x2), y3 = f(x3);
        const double D = (x1 - x2) * (x1 - x3) * (x2 - x3);
        if (fabs(D) < 1e-18) break;

        const double A = (y1 * (x2 - x3) + y2 * (x3 - x1) + y3 * (x1 - x2)) / D;
        const double Bc = (y1 * (x3 * x3 - x2 * x2) + y2 * (x1 * x1 - x3 * x3) + y3 * (x2 * x2 - x1 * x1)) / D;

        if (fabs(A) < 1e-18) break;

        const double xm = -Bc / (2.0 * A);
        if (!std::isfinite(xm)) break;

        const double xmid = std::min(x3, std::max(x1, xm));
        const double h = 0.6 * 0.5 * (x3 - x1);
        x1 = std::max(1e-6, xmid - h);
        x2 = std::min(1.0 - 1e-6, std::max(1e-6, xmid));
        x3 = std::min(1.0 - 1e-6, xmid + h);
    }
    return x2;
}
