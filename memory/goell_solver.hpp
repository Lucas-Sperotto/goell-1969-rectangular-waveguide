#ifndef GOELL_SOLVER_HPP
#define GOELL_SOLVER_HPP

#include "goell_common.hpp" // For Params
#include "goell_matrix.hpp" // For DeterminantInfo
#include <vector>
#include <algorithm> // For std::min, std::max
#include <cmath>     // For isfinite

// Structs for scan results
struct Sample
{
    double B = 0.0;
    double Pprime = 0.0;
    double merit = 0.0;
};

struct DetSample
{
    double B = 0.0;
    double Pprime = 0.0;
    double merit = 0.0;
    int sign = 0;
};

// Core solver functions
double merit_value(const Params &P, double B, double Pprime);
DeterminantInfo determinant_value(const Params &P, double B, double Pprime);
std::vector<Sample> scan_P(const Params &P, double B);
std::vector<DetSample> scan_P_det(const Params &P, double B);
std::vector<Sample> local_minima(const std::vector<Sample> &samples);
std::vector<Sample> edge_minima(const std::vector<Sample> &samples);
std::vector<Sample> sign_change_roots(const Params &P, double B, const std::vector<DetSample> &samples);
double refine_local_minimum(const Params &P, double B, double x0);

#endif // GOELL_SOLVER_HPP
