#include "goell/boundary.hpp"
#include "goell/cli.hpp"
#include "goell/diagnostics.hpp"
#include "goell/layout.hpp"
#include "goell/matrix.hpp"

#include <cmath>
#include <iostream>
#include <string>

namespace
{
bool expect(bool condition, const std::string &message)
{
    if (!condition)
    {
        std::cerr << "[check-cpp] falhou: " << message << "\n";
        return false;
    }
    return true;
}
} // namespace

int main()
{
    bool ok = true;

    {
        Params P;
        char arg0[] = "goell_q_solver";
        char arg1[] = "--parity";
        char arg2[] = "odd";
        char arg3[] = "--phase";
        char arg4[] = "phi90";
        char arg5[] = "--N";
        char arg6[] = "3";
        char *argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6};

        parse_args(7, argv, P);
        ok &= expect(P.parity == HarmonicParity::odd, "parse_args deve reconhecer parity odd");
        ok &= expect(P.phase == PhaseFamily::phi90, "parse_args deve reconhecer phase phi90");
        ok &= expect(P.N == 3, "parse_args deve reconhecer N=3");

        const ColumnLayout L = build_layout(P);
        ok &= expect(static_cast<int>(L.ez_orders.size()) == 3, "layout odd deve ter 3 harmônicos Ez");
        ok &= expect(static_cast<int>(L.hz_orders.size()) == 3, "layout odd deve ter 3 harmônicos Hz");
        ok &= expect(L.ncols == 12, "layout odd com N=3 deve produzir 12 colunas");

        const auto bp = boundary_point(0.2, 2.0, BoundaryGeometryMode::intersection);
        ok &= expect(bp.r > 0.0, "boundary_point deve retornar r positivo");

        const auto Q = assemble_Q(P, 2.0, 0.7);
        ok &= expect(Q.rows() == 12 && Q.cols() == 12, "assemble_Q odd com N=3 deve ser 12x12");

        const auto det = determinant_info(Q);
        ok &= expect(std::isfinite(det.logabs), "determinant_info deve retornar logabs finito em ponto generico");

        const auto null_info = compute_null_info(P, 2.0, 0.7);
        ok &= expect(null_info.ez_frac >= 0.0 && null_info.ez_frac <= 1.0, "Ez_frac deve ficar em [0,1]");
        ok &= expect(null_info.hz_frac >= 0.0 && null_info.hz_frac <= 1.0, "Hz_frac deve ficar em [0,1]");
    }

    {
        Params P;
        P.parity = HarmonicParity::even;
        P.phase = PhaseFamily::phi0;
        P.N = 4;

        const ColumnLayout L = build_layout(P);
        ok &= expect(static_cast<int>(L.ez_orders.size()) == 3, "layout even/phi0 deve remover o n=0 de Ez");
        ok &= expect(static_cast<int>(L.hz_orders.size()) == 4, "layout even/phi0 deve manter 4 harmônicos Hz");
        ok &= expect(L.ncols == 14, "layout even/phi0 com N=4 deve produzir 14 colunas");

        const auto Q = assemble_Q(P, 2.0, 0.7);
        ok &= expect(Q.rows() == 14 && Q.cols() == 14, "assemble_Q even com N=4 deve ser 14x14");
    }

    if (!ok)
        return 1;

    std::cout << "[check-cpp] PASS\n";
    return 0;
}
