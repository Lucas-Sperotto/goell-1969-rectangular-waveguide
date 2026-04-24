#ifndef GOELL_COMMON_HPP
#define GOELL_COMMON_HPP

#include <string>

static constexpr double PI = 3.14159265358979323846;
static constexpr double EPS = 1e-12;

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
    bool null_vector = false;
};

std::string parity_name(HarmonicParity parity);
std::string phase_name(PhaseFamily phase);
std::string geometry_name(BoundaryGeometryMode mode);

#endif // GOELL_COMMON_HPP
