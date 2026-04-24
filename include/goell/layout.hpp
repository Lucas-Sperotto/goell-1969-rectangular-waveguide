#ifndef GOELL_LAYOUT_HPP
#define GOELL_LAYOUT_HPP

#include "goell/common.hpp"

#include <vector>

struct ColumnLayout
{
    std::vector<int> ez_orders;
    std::vector<int> hz_orders;

    int offset_A = 0;
    int offset_B = 0;
    int offset_C = 0;
    int offset_D = 0;
    int ncols = 0;
};

std::vector<int> odd_orders(int N);
std::vector<int> even_orders(int N);
std::vector<int> drop_zero(const std::vector<int> &orders);
double S_term(int n, double theta, PhaseFamily phase);
double C_term(int n, double theta, PhaseFamily phase);
ColumnLayout build_layout(const Params &P);

#endif // GOELL_LAYOUT_HPP
