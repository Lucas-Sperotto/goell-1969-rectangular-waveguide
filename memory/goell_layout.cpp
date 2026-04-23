#include "goell_layout.hpp"

std::vector<int> odd_orders(int N)
{
    std::vector<int> out;
    out.reserve(N);
    for (int k = 0; k < N; ++k)
        out.push_back(2 * k + 1);
    return out;
}

std::vector<int> even_orders(int N)
{
    std::vector<int> out;
    out.reserve(N);
    for (int k = 0; k < N; ++k)
        out.push_back(2 * k);
    return out;
}

std::vector<int> drop_zero(const std::vector<int> &orders)
{
    std::vector<int> out;
    for (int n : orders)
        if (n != 0)
            out.push_back(n);
    return out;
}

// Sec. 2.1:
// - phi=0    -> S = sin(n theta),   C = cos(n theta)
// - phi=pi/2 -> S = cos(n theta),   C = -sin(n theta)
double S_term(int n, double theta, PhaseFamily phase)
{
    return (phase == PhaseFamily::phi0) ? sin(n * theta) : cos(n * theta);
}

double C_term(int n, double theta, PhaseFamily phase)
{
    return (phase == PhaseFamily::phi0) ? cos(n * theta) : -sin(n * theta);
}

ColumnLayout build_layout(const Params &P)
{
    ColumnLayout L;

    if (P.parity == HarmonicParity::odd) { L.ez_orders = odd_orders(P.N); L.hz_orders = L.ez_orders; }
    else if (P.phase == PhaseFamily::phi0) { L.ez_orders = drop_zero(even_orders(P.N)); L.hz_orders = even_orders(P.N); }
    else { L.ez_orders = even_orders(P.N); L.hz_orders = drop_zero(even_orders(P.N)); }

    const int nA = static_cast<int>(L.ez_orders.size());
    const int nB = static_cast<int>(L.hz_orders.size());

    L.offset_A = 0;
    L.offset_B = L.offset_A + nA;
    L.offset_C = L.offset_B + nB;
    L.offset_D = L.offset_C + nA;
    L.ncols = L.offset_D + nB;

    return L;
}