#ifndef GOELL_MATRIX_HPP
#define GOELL_MATRIX_HPP

#include "goell_common.hpp"  // For Params, RowKind
#include "goell_layout.hpp"  // For ColumnLayout
#include "goell_boundary.hpp" // For BoundaryPoint
#include <Eigen/Dense>
#include <cmath> // For log, fabs, isfinite

// Struct to hold determinant information
struct DeterminantInfo
{
    double logabs = -INFINITY;
    int sign = 0;
};

// Helper for safe division
double safe_denominator(double x);

// Matrix scaling functions
void row_rescale(Eigen::MatrixXd &Q);
void col_rescale(Eigen::MatrixXd &Q);

// Functions to fill matrix columns
void fill_ez_columns(Eigen::MatrixXd &Q, int row, RowKind row_kind, const ColumnLayout &L,
                     const Params &P, const BoundaryPoint &bp, double kz_over_k0,
                     double h_scaled, double p_scaled);
void fill_hz_columns(Eigen::MatrixXd &Q, int row, RowKind row_kind, const ColumnLayout &L,
                     const Params &P, const BoundaryPoint &bp, double kz_over_k0,
                     double h_scaled, double p_scaled);
void append_row(Eigen::MatrixXd &Q, int row, RowKind row_kind, const ColumnLayout &L,
                const Params &P, const BoundaryPoint &bp, double kz_over_k0,
                double h_scaled, double p_scaled);

// Main matrix assembly function
Eigen::MatrixXd assemble_Q(const Params &P, double B, double Pprime);
DeterminantInfo determinant_info(const Eigen::MatrixXd &Q);
double log10_sigma_rel(const Eigen::MatrixXd &Q);

#endif // GOELL_MATRIX_HPP