#ifndef GOELL_MATRIX_HPP
#define GOELL_MATRIX_HPP

#include "goell/common.hpp"

#include <Eigen/Dense>

Eigen::MatrixXd assemble_Q(const Params &P, double B, double Pprime);

#endif // GOELL_MATRIX_HPP
