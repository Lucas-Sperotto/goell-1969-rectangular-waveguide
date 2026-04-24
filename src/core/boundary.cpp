#include "goell/boundary.hpp"

#include <cmath>

BoundaryPoint boundary_point(double theta, double a_over_b, BoundaryGeometryMode geometry_mode)
{
    const double theta_c = std::atan(1.0 / a_over_b);

    BoundaryPoint bp;
    bp.theta = theta;

    if (theta < theta_c)
    {
        if (geometry_mode == BoundaryGeometryMode::literal)
        {
            bp.r = (0.5 * a_over_b) * std::cos(theta);
        }
        else
        {
            bp.r = (0.5 * a_over_b) / std::cos(theta);
        }
        bp.R = std::sin(theta);
        bp.T = std::cos(theta);
    }
    else
    {
        if (geometry_mode == BoundaryGeometryMode::literal)
        {
            bp.r = 0.5 * std::sin(theta);
        }
        else
        {
            bp.r = 0.5 / std::sin(theta);
        }
        bp.R = -std::cos(theta);
        bp.T = std::sin(theta);
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
    if (N <= 1)
        return thetas;
    thetas.reserve(N - 1);
    for (int m = 1; m <= N - 1; ++m)
        thetas.push_back((m - 0.5) * PI / (2.0 * (N - 1)));
    return thetas;
}

std::vector<double> omit_first_last(const std::vector<double> &thetas)
{
    if (thetas.size() <= 2)
        return {};
    return std::vector<double>(thetas.begin() + 1, thetas.end() - 1);
}

bool uses_even_symmetry_points(RowKind row_kind, PhaseFamily phase)
{
    if (phase == PhaseFamily::phi0)
        return row_kind == RowKind::hz_long || row_kind == RowKind::et_tan;
    return row_kind == RowKind::ez_long || row_kind == RowKind::ht_tan;
}

RowKind odd_z_row_kind(PhaseFamily phase)
{
    return (phase == PhaseFamily::phi0) ? RowKind::ez_long : RowKind::hz_long;
}
