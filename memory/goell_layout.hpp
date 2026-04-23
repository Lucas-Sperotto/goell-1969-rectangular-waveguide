#ifndef GOELL_LAYOUT_HPP
#define GOELL_LAYOUT_HPP

#include "goell_common.hpp" // For HarmonicParity, PhaseFamily
#include <vector>
#include <cmath> // For sin, cos

// Struct to define the column layout of the Q matrix
struct ColumnLayout
{
    std::vector<int> ez_orders; // orders n for a_n and c_n coefficients
    std::vector<int> hz_orders; // orders n for b_n and d_n coefficients

    int offset_A = 0;
    int offset_B = 0;
    int offset_C = 0;
    int offset_D = 0;
    int ncols = 0;
};

// Functions to generate harmonic orders
std::vector<int> odd_orders(int N);
std::vector<int> even_orders(int N);
std::vector<int> drop_zero(const std::vector<int> &orders);

// Functions for symmetry terms (S and C from Sec. 2.1)
double S_term(int n, double theta, PhaseFamily phase);
double C_term(int n, double theta, PhaseFamily phase);
ColumnLayout build_layout(const Params &P);

#endif // GOELL_LAYOUT_HPP