#ifndef GOELL_BOUNDARY_HPP
#define GOELL_BOUNDARY_HPP

#include "goell/common.hpp"

#include <vector>

struct BoundaryPoint
{
    double theta = 0.0;
    double r = 0.0;
    double R = 0.0;
    double T = 0.0;
};

BoundaryPoint boundary_point(double theta, double a_over_b, BoundaryGeometryMode geometry_mode);
std::vector<double> odd_case_thetas(int N);
std::vector<double> even_symmetry_thetas(int N);
std::vector<double> odd_symmetry_thetas(int N);
std::vector<double> omit_first_last(const std::vector<double> &thetas);
bool uses_even_symmetry_points(RowKind row_kind, PhaseFamily phase);
RowKind odd_z_row_kind(PhaseFamily phase);

#endif // GOELL_BOUNDARY_HPP
