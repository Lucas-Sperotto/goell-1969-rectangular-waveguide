#ifndef GOELL_BESSEL_HPP
#define GOELL_BESSEL_HPP

#include <cmath> // For std::cyl_bessel_j, std::cyl_bessel_k

// Bessel functions and their derivatives
double Jn(int n, double x);
double Kn(int n, double x);
double Jn_prime(int n, double x);
double Kn_prime(int n, double x);

#endif // GOELL_BESSEL_HPP