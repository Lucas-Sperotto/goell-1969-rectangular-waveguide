#include "goell_bessel.hpp"

double Jn(int n, double x) { return std::cyl_bessel_j(n, x); }
double Kn(int n, double x) { return std::cyl_bessel_k(n, x); }

double Jn_prime(int n, double x)
{
    if (n == 0)
        return -std::cyl_bessel_j(1, x);
    // Recurrence relation: J_n'(x) = 0.5 * (J_{n-1}(x) - J_{n+1}(x))
    return 0.5 * (std::cyl_bessel_j(n - 1, x) - std::cyl_bessel_j(n + 1, x));
}

double Kn_prime(int n, double x)
{
    if (n == 0)
        return -std::cyl_bessel_k(1, x);
    // Recurrence relation: K_n'(x) = -0.5 * (K_{n-1}(x) + K_{n+1}(x))
    return -0.5 * (std::cyl_bessel_k(n - 1, x) + std::cyl_bessel_k(n + 1, x));
}