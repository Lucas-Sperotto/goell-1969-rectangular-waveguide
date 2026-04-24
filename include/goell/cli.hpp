#ifndef GOELL_CLI_HPP
#define GOELL_CLI_HPP

#include "goell/common.hpp"

#include <string>

HarmonicParity parse_parity(const std::string &value);
PhaseFamily parse_phase(const std::string &value);
std::string parse_det_search_mode(const std::string &value);
BoundaryGeometryMode parse_geometry_mode(const std::string &value);
EvenRectMatchingMode parse_even_rect_mode(const std::string &value);
void parse_args(int argc, char **argv, Params &P);

#endif // GOELL_CLI_HPP
