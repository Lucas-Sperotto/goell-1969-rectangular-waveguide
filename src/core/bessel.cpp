#include "goell/bessel.hpp"

double Jn(int n, double x) { return std::cyl_bessel_j(n, x); }
double Kn(int n, double x) { return std::cyl_bessel_k(n, x); }

double Jn_prime(int n, double x)
{
    if (n == 0)
        return -std::cyl_bessel_j(1, x);
    return 0.5 * (std::cyl_bessel_j(n - 1, x) - std::cyl_bessel_j(n + 1, x));
}

double Kn_prime(int n, double x)
{
    if (n == 0)
        return -std::cyl_bessel_k(1, x);
    return -0.5 * (std::cyl_bessel_k(n - 1, x) + std::cyl_bessel_k(n + 1, x));
}
