#include "goell_matrix.hpp"
#include "goell_bessel.hpp" // For Jn, Kn, Jn_prime, Kn_prime
#include <algorithm> // For std::max

using namespace Eigen;

double safe_denominator(double x)
{
    return (x < EPS) ? EPS : x;
}

void row_rescale(MatrixXd &Q)
{
    for (int i = 0; i < Q.rows(); ++i)
    {
        double max_abs = 0.0;
        for (int j = 0; j < Q.cols(); ++j)
            max_abs = std::max(max_abs, fabs(Q(i, j)));
        if (max_abs > 0.0)
            Q.row(i) /= max_abs;
    }
}

void col_rescale(MatrixXd &Q)
{
    for (int j = 0; j < Q.cols(); ++j)
    {
        double max_abs = 0.0;
        for (int i = 0; i < Q.rows(); ++i)
            max_abs = std::max(max_abs, fabs(Q(i, j)));
        if (max_abs > 0.0)
            Q.col(j) /= max_abs;
    }
}

void fill_ez_columns(
    MatrixXd &Q,
    int row,
    RowKind row_kind,
    const ColumnLayout &L,
    const Params &P,
    const BoundaryPoint &bp,
    double kz_over_k0,
    double h_scaled,
    double p_scaled)
{
    const double eps_r = P.n_r * P.n_r;

    for (int i = 0; i < static_cast<int>(L.ez_orders.size()); ++i)
    {
        const int n = L.ez_orders[i];
        const double S = S_term(n, bp.theta, P.phase);
        const double C = C_term(n, bp.theta, P.phase);

        const double hr = h_scaled * bp.r;
        const double pr = p_scaled * bp.r;

        const double J0 = Jn(n, hr);
        const double K0 = Kn(n, pr);

        const double Jr  = Jn_prime(n, hr) / safe_denominator(h_scaled);
        const double Kr  = Kn_prime(n, pr) / safe_denominator(p_scaled);
        const double Jth = (n == 0) ? 0.0 : (n * J0) / (safe_denominator(h_scaled * h_scaled) * bp.r);
        const double Kth = (n == 0) ? 0.0 : (n * K0) / (safe_denominator(p_scaled * p_scaled) * bp.r);

        const double eLA = J0 * S;
        const double eLC = K0 * S;

        const double eTA = -kz_over_k0 * (Jr  * S * bp.R + Jth * C * bp.T);
        const double eTC = +kz_over_k0 * (Kr  * S * bp.R + Kth * C * bp.T);

        const double hTA = +eps_r * (Jth * C * bp.R - Jr  * S * bp.T);
        const double hTC = -(Kth * C * bp.R - Kr  * S * bp.T);

        const int colA = L.offset_A + i;
        const int colC = L.offset_C + i;

        if (row_kind == RowKind::ez_long) { Q(row, colA) = eLA; Q(row, colC) = -eLC; }
        else if (row_kind == RowKind::et_tan) { Q(row, colA) = eTA; Q(row, colC) = -eTC; }
        else if (row_kind == RowKind::ht_tan) { Q(row, colA) = hTA; Q(row, colC) = -hTC; }
    }
}

void fill_hz_columns(
    MatrixXd &Q,
    int row,
    RowKind row_kind,
    const ColumnLayout &L,
    const Params &P,
    const BoundaryPoint &bp,
    double kz_over_k0,
    double h_scaled,
    double p_scaled)
{
    for (int i = 0; i < static_cast<int>(L.hz_orders.size()); ++i)
    {
        const int n = L.hz_orders[i];
        const double S = S_term(n, bp.theta, P.phase);
        const double C = C_term(n, bp.theta, P.phase);

        const double hr = h_scaled * bp.r;
        const double pr = p_scaled * bp.r;

        const double J0 = Jn(n, hr);
        const double K0 = Kn(n, pr);

        const double Jr  = Jn_prime(n, hr) / safe_denominator(h_scaled);
        const double Kr  = Kn_prime(n, pr) / safe_denominator(p_scaled);
        const double Jth = (n == 0) ? 0.0 : (n * J0) / (safe_denominator(h_scaled * h_scaled) * bp.r);
        const double Kth = (n == 0) ? 0.0 : (n * K0) / (safe_denominator(p_scaled * p_scaled) * bp.r);

        const double hLB = J0 * C;
        const double hLD = K0 * C;

        const double eTB = +(Jth * S * bp.R + Jr  * C * bp.T);
        const double eTD = -(Kth * S * bp.R + Kr  * C * bp.T);

        const double hTB = -kz_over_k0 * (Jr  * C * bp.R - Jth * S * bp.T);
        const double hTD = +kz_over_k0 * (Kr  * C * bp.R - Kth * S * bp.T);

        const int colB = L.offset_B + i;
        const int colD = L.offset_D + i;

        if (row_kind == RowKind::hz_long) { Q(row, colB) = hLB; Q(row, colD) = -hLD; }
        else if (row_kind == RowKind::et_tan) { Q(row, colB) = eTB; Q(row, colD) = -eTD; }
        else if (row_kind == RowKind::ht_tan) { Q(row, colB) = hTB; Q(row, colD) = -hTD; }
    }
}

void append_row(
    MatrixXd &Q,
    int row,
    RowKind row_kind,
    const ColumnLayout &L,
    const Params &P,
    const BoundaryPoint &bp,
    double kz_over_k0,
    double h_scaled,
    double p_scaled)
{
    fill_ez_columns(Q, row, row_kind, L, P, bp, kz_over_k0, h_scaled, p_scaled);
    fill_hz_columns(Q, row, row_kind, L, P, bp, kz_over_k0, h_scaled, p_scaled);
}

MatrixXd assemble_Q(const Params &P, double B, double Pprime)
{
    const ColumnLayout L = build_layout(P);

    const double eps_r = P.n_r * P.n_r;
    const double kz_over_k0 = sqrt(1.0 + (eps_r - 1.0) * Pprime);

    const double radial_scale = PI * std::max(0.0, B);
    const double h_scaled = radial_scale * sqrt(std::max(0.0, 1.0 - Pprime));
    const double p_scaled = radial_scale * sqrt(std::max(0.0, Pprime));

    MatrixXd Q;

    if (P.parity == HarmonicParity::odd)
    {
        const auto thetas = odd_case_thetas(P.N);
        Q = MatrixXd::Zero(4 * P.N, L.ncols);

        int row = 0;
        for (RowKind kind : {RowKind::ez_long, RowKind::hz_long, RowKind::et_tan, RowKind::ht_tan})
        {
            for (double theta : thetas)
            {
                const auto bp = boundary_point(theta, P.a_over_b, P.geometry_mode);
                append_row(Q, row++, kind, L, P, bp, kz_over_k0, h_scaled, p_scaled);
            }
        }
    }
    else
    {
        const auto theta_full = even_symmetry_thetas(P.N);
        const auto theta_odd_square = odd_symmetry_thetas(P.N);
        const auto theta_odd_rect = omit_first_last(theta_full);
        const bool is_square = fabs(P.a_over_b - 1.0) <= 1e-12;

        Q = MatrixXd::Zero(4 * P.N - 2, L.ncols);

        int row = 0;
        for (RowKind kind : {RowKind::ez_long, RowKind::hz_long, RowKind::et_tan, RowKind::ht_tan})
        {
            const std::vector<double> *thetas = nullptr;
            if (is_square)
            {
                const bool use_even_points = uses_even_symmetry_points(kind, P.phase);
                thetas = use_even_points ? &theta_full : &theta_odd_square;
            }
            else
            {
                if (P.even_rect_mode == EvenRectMatchingMode::paper)
                {
                    thetas = (kind == odd_z_row_kind(P.phase)) ? &theta_odd_rect : &theta_full;
                }
                else
                {
                    const bool use_even_points = uses_even_symmetry_points(kind, P.phase);
                    thetas = use_even_points ? &theta_full : &theta_odd_square;
                }
            }

            for (double theta : *thetas)
            {
                const auto bp = boundary_point(theta, P.a_over_b, P.geometry_mode);
                append_row(Q, row++, kind, L, P, bp, kz_over_k0, h_scaled, p_scaled);
            }
        }
    }

    if (P.rescale_matrix)
    {
        row_rescale(Q);
        col_rescale(Q);
    }

    return Q;
}

DeterminantInfo determinant_info(const MatrixXd &Q)
{
    FullPivLU<MatrixXd> lu(Q);
    const auto &LU = lu.matrixLU();

    DeterminantInfo info;
    info.logabs = 0.0;

    int sign = static_cast<int>(std::round(lu.permutationP().determinant() * lu.permutationQ().determinant()));
    if (sign == 0) sign = 1;

    for (int i = 0; i < LU.rows(); ++i)
    {
        const double di = LU(i, i);
        if (!std::isfinite(di) || fabs(di) <= 0.0)
        {
            info.logabs = -INFINITY;
            info.sign = 0;
            return info;
        }

        info.logabs += log(fabs(di));
        sign *= (di < 0.0) ? -1 : 1;
    }

    info.sign = sign;
    return info;
}

double log10_sigma_rel(const MatrixXd &Q)
{
    JacobiSVD<MatrixXd> svd(Q, ComputeThinU | ComputeThinV);
    const auto s = svd.singularValues();
    if (s.size() == 0 || s(0) <= 0.0)
        return 0.0;
    return log10(s.tail(1)(0) / s(0));
}
