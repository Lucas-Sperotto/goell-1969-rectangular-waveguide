#include <exception>
#include <iomanip>
#include <iostream>

#include "goell/cli.hpp"
#include "goell/common.hpp"
#include "goell/output.hpp"

int main(int argc, char **argv)
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Params P;
    try
    {
        parse_args(argc, argv, P);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[goell_q_solver] " << e.what() << "\n";
        return 1;
    }

    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(15);

    if (P.test_bessel)
    {
        write_bessel_csv(std::cout);
        return 0;
    }

    std::cout.precision(6);

    if (P.dump_scan)
    {
        write_dump_scan_csv(std::cout, P);
        return 0;
    }

    write_root_csv(std::cout, P);
    return 0;
}
