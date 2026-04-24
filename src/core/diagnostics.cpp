#include "goell/diagnostics.hpp"

#include "goell/layout.hpp"
#include "goell/matrix.hpp"

#include <Eigen/Dense>

#include <cmath>

using Eigen::ComputeThinU;
using Eigen::ComputeThinV;
using Eigen::FullPivLU;
using Eigen::JacobiSVD;
using Eigen::MatrixXd;
using Eigen::VectorXd;

double logabs_det(const MatrixXd &Q)
{
    FullPivLU<MatrixXd> lu(Q);
    const auto &LU = lu.matrixLU();
    double sum = 0.0;

    for (int i = 0; i < LU.rows(); ++i)
    {
        const double diagonal = std::fabs(LU(i, i));
        if (!(diagonal > 0.0) || !std::isfinite(diagonal))
            return -std::numeric_limits<double>::infinity();
        sum += std::log(diagonal);
    }

    return sum;
}

DeterminantInfo determinant_info(const MatrixXd &Q)
{
    FullPivLU<MatrixXd> lu(Q);
    const auto &LU = lu.matrixLU();

    DeterminantInfo info;
    info.logabs = 0.0;

    int sign = static_cast<int>(
        std::round(lu.permutationP().determinant() * lu.permutationQ().determinant()));
    if (sign == 0)
        sign = 1;

    for (int i = 0; i < LU.rows(); ++i)
    {
        const double diagonal = LU(i, i);
        if (!std::isfinite(diagonal) || std::fabs(diagonal) <= 0.0)
        {
            info.logabs = -std::numeric_limits<double>::infinity();
            info.sign = 0;
            return info;
        }

        info.logabs += std::log(std::fabs(diagonal));
        sign *= (diagonal < 0.0) ? -1 : 1;
    }

    info.sign = sign;
    return info;
}

double log10_sigma_rel(const MatrixXd &Q)
{
    JacobiSVD<MatrixXd> svd(Q, ComputeThinU | ComputeThinV);
    const auto singular_values = svd.singularValues();
    if (singular_values.size() == 0 || singular_values(0) <= 0.0)
        return 0.0;
    return std::log10(singular_values.tail(1)(0) / singular_values(0));
}

NullInfo compute_null_info(const Params &P, double B, double Pprime)
{
    const ColumnLayout layout = build_layout(P);
    const MatrixXd Q = assemble_Q(P, B, Pprime);

    NullInfo info;
    if (Q.cols() == 0)
        return info;

    JacobiSVD<MatrixXd> svd(Q, ComputeThinV);
    if (svd.matrixV().cols() == 0)
        return info;

    const VectorXd v = svd.matrixV().col(svd.matrixV().cols() - 1);
    const int nA = static_cast<int>(layout.ez_orders.size());
    const int nB = static_cast<int>(layout.hz_orders.size());

    double ez_energy = 0.0;
    for (int j = layout.offset_A; j < layout.offset_A + nA; ++j)
        ez_energy += v(j) * v(j);
    for (int j = layout.offset_C; j < layout.offset_C + nA; ++j)
        ez_energy += v(j) * v(j);

    double hz_energy = 0.0;
    for (int j = layout.offset_B; j < layout.offset_B + nB; ++j)
        hz_energy += v(j) * v(j);
    for (int j = layout.offset_D; j < layout.offset_D + nB; ++j)
        hz_energy += v(j) * v(j);

    const double total = ez_energy + hz_energy;
    if (total <= 0.0)
        return info;

    info.ez_frac = ez_energy / total;
    info.hz_frac = hz_energy / total;

    if (info.ez_frac > 0.7)
        info.mode_class = "EH";
    else if (info.hz_frac > 0.7)
        info.mode_class = "HE";
    else
        info.mode_class = "hybrid";

    return info;
}
