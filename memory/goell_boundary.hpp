#ifndef GOELL_BOUNDARY_HPP
#define GOELL_BOUNDARY_HPP

#include "goell_common.hpp" // For BoundaryGeometryMode, PhaseFamily, RowKind, Params
#include <vector>
#include <cmath> // For atan, sin, cos

// Struct to represent a boundary point
struct BoundaryPoint
{
    double theta = 0.0;
    double r = 0.0; // r_m normalized by b
    double R = 0.0; // tangential projection of radial component
    double T = 0.0; // tangential projection of azimuthal component
};

// Function to calculate a boundary point
BoundaryPoint boundary_point(double theta, double a_over_b, BoundaryGeometryMode geometry_mode);

// Functions to generate theta points for matching
std::vector<double> odd_case_thetas(int N);
std::vector<double> even_symmetry_thetas(int N);
std::vector<double> odd_symmetry_thetas(int N);
std::vector<double> omit_first_last(const std::vector<double> &thetas);

// Function to determine which set of symmetry points to use
bool uses_even_symmetry_points(RowKind row_kind, PhaseFamily phase);

// Function to determine the "odd" z-component row kind based on phase
RowKind odd_z_row_kind(PhaseFamily phase);

#endif // GOELL_BOUNDARY_HPP