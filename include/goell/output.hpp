#ifndef GOELL_OUTPUT_HPP
#define GOELL_OUTPUT_HPP

#include "goell/common.hpp"

#include <iosfwd>

void write_bessel_csv(std::ostream &out);
void write_dump_scan_csv(std::ostream &out, const Params &P);
void write_root_csv(std::ostream &out, const Params &P);

#endif // GOELL_OUTPUT_HPP
