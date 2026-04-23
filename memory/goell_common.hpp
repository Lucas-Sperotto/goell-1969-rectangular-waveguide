#ifndef GOELL_COMMON_HPP
#define GOELL_COMMON_HPP

#include <string>
#include <vector>
#include <stdexcept> // For runtime_error

// Global Constants
static constexpr double PI = 3.14159265358979323846;
static constexpr double EPS = 1e-12;

// Enums
enum class HarmonicParity
{
    odd,
    even,
};

enum class PhaseFamily
{
    phi0,
    phi90,
};

enum class BoundaryGeometryMode
{
    literal,
    intersection,
};

enum class EvenRectMatchingMode
{
    paper,
    square_split,
};

enum class RowKind
{
    ez_long,
    hz_long,
    et_tan,
    ht_tan,
};

// Struct for solver parameters
struct Params
{
    double a_over_b = 1.0;
    double n_r = 1.01;
    int N = 5;
    double B_min = 0.1;
    double B_max = 4.0;
    int NB = 40;
    int Pscan = 160;
    HarmonicParity parity = HarmonicParity::odd;
    PhaseFamily phase = PhaseFamily::phi0;
    BoundaryGeometryMode geometry_mode = BoundaryGeometryMode::intersection;
    EvenRectMatchingMode even_rect_mode = EvenRectMatchingMode::paper;
    std::string metric = "det";
    std::string det_search = "sign";
    bool all_minima = true;
    bool allow_edge_minima = false;
    bool rescale_matrix = true;
    bool dump_scan = false;
    double dump_B = 0.0;
    bool dump_det_sign = false;
    bool test_bessel = false;
};

// Argument parsing and string conversion functions
std::string parity_name(HarmonicParity parity);
std::string phase_name(PhaseFamily phase);
std::string geometry_name(BoundaryGeometryMode mode);
HarmonicParity parse_parity(const std::string &value);
PhaseFamily parse_phase(const std::string &value);
std::string parse_det_search_mode(const std::string &value);
BoundaryGeometryMode parse_geometry_mode(const std::string &value);
EvenRectMatchingMode parse_even_rect_mode(const std::string &value);
void parse_args(int argc, char **argv, Params &P);

#endif // GOELL_COMMON_HPP