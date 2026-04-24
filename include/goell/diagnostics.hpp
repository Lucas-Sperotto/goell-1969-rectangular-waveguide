#ifndef GOELL_DIAGNOSTICS_HPP
#define GOELL_DIAGNOSTICS_HPP

#include "goell/common.hpp"

#include <Eigen/Dense>
#include <limits>
#include <string>

struct DeterminantInfo
{
    double logabs = -std::numeric_limits<double>::infinity();
    int sign = 0;
};

struct NullInfo
{
    double ez_frac = 0.5;
    double hz_frac = 0.5;
    std::string mode_class = "hybrid";
};

double logabs_det(const Eigen::MatrixXd &Q);
DeterminantInfo determinant_info(const Eigen::MatrixXd &Q);
double log10_sigma_rel(const Eigen::MatrixXd &Q);
NullInfo compute_null_info(const Params &P, double B, double Pprime);

#endif // GOELL_DIAGNOSTICS_HPP
