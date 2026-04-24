#include "goell/common.hpp"

std::string parity_name(HarmonicParity parity)
{
    return (parity == HarmonicParity::odd) ? "odd" : "even";
}

std::string phase_name(PhaseFamily phase)
{
    return (phase == PhaseFamily::phi0) ? "phi0" : "phi90";
}

std::string geometry_name(BoundaryGeometryMode mode)
{
    return (mode == BoundaryGeometryMode::literal) ? "literal" : "intersection";
}
