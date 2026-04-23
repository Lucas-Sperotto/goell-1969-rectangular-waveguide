#include "goell_boundary.hpp"

// Geometria da Sec. 2 e definicoes abaixo das eqs. (7):
// - usamos a e b como dimensoes completas do paper
// - normalizamos por b, logo o retangulo fica em
//     x in [-a/(2b), +a/(2b)] = [-a_over_b/2, +a_over_b/2]
//     y in [-1/2, +1/2]
//
// geometry_mode:
// - literal: segue literalmente a leitura consolidada nos .md
// - intersection: usa a intersecao geometrica da reta radial com a fronteira
//   do retangulo, isto e, sec/csc. Este modo existe para validacao numerica,
//   pois ele parece mais consistente com a geometria da Fig. 2.
BoundaryPoint boundary_point(double theta, double a_over_b, BoundaryGeometryMode geometry_mode)
{
    const double theta_c = atan(1.0 / a_over_b);

    BoundaryPoint bp;
    bp.theta = theta;

    if (theta < theta_c)
    {
        if (geometry_mode == BoundaryGeometryMode::literal)
        {
            // Leitura literal consolidada nos .md:
            //   r_m = (a/2) cos(theta_m)
            bp.r = (0.5 * a_over_b) * cos(theta);
        }
        else
        {
            // Intersecao com o lado vertical x = a/2:
            //   r_m cos(theta_m) = a/2  =>  r_m = (a/2) sec(theta_m)
            bp.r = (0.5 * a_over_b) / cos(theta);
        }
        bp.R = sin(theta);
        bp.T = cos(theta);
    }
    else
    {
        if (geometry_mode == BoundaryGeometryMode::literal)
        {
            // Leitura literal consolidada nos .md:
            //   r_m = (b/2) sin(theta_m)
            bp.r = 0.5 * sin(theta);
        }
        else
        {
            // Intersecao com o lado horizontal y = b/2:
            //   r_m sin(theta_m) = b/2  =>  r_m = (b/2) csc(theta_m)
            bp.r = 0.5 / sin(theta);
        }
        bp.R = -cos(theta);
        bp.T = sin(theta);
    }

    return bp;
}

std::vector<double> odd_case_thetas(int N)
{
    std::vector<double> thetas;
    thetas.reserve(N);
    for (int m = 1; m <= N; ++m)
        thetas.push_back((m - 0.5) * PI / (2.0 * N));
    return thetas;
}

std::vector<double> even_symmetry_thetas(int N)
{
    std::vector<double> thetas;
    thetas.reserve(N);
    for (int m = 1; m <= N; ++m)
        thetas.push_back((m - 0.5) * PI / (2.0 * N));
    return thetas;
}

std::vector<double> odd_symmetry_thetas(int N)
{
    std::vector<double> thetas;
    if (N <= 1) return thetas;
    thetas.reserve(N - 1);
    for (int m = 1; m <= N - 1; ++m)
        thetas.push_back((m - 0.5) * PI / (2.0 * (N - 1)));
    return thetas;
}

std::vector<double> omit_first_last(const std::vector<double> &thetas)
{
    if (thetas.size() <= 2) return {};
    return std::vector<double>(thetas.begin() + 1, thetas.end() - 1);
}

bool uses_even_symmetry_points(RowKind row_kind, PhaseFamily phase)
{
    if (phase == PhaseFamily::phi0) return row_kind == RowKind::hz_long || row_kind == RowKind::et_tan;
    return row_kind == RowKind::ez_long || row_kind == RowKind::ht_tan;
}

RowKind odd_z_row_kind(PhaseFamily phase)
{
    return (phase == PhaseFamily::phi0) ? RowKind::ez_long : RowKind::hz_long;
}