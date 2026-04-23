#include "goell_common.hpp"
#include <string>
#include <vector>
#include <cstdlib> // For atof, atoi

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

HarmonicParity parse_parity(const std::string &value)
{
    if (value == "odd")
        return HarmonicParity::odd;
    if (value == "even")
        return HarmonicParity::even;
    throw std::runtime_error("parity invalida; use odd ou even");
}

PhaseFamily parse_phase(const std::string &value)
{
    if (value == "phi0" || value == "0")
        return PhaseFamily::phi0;
    if (value == "phi90" || value == "pi/2" || value == "pi2")
        return PhaseFamily::phi90;
    throw std::runtime_error("phase invalida; use phi0 ou phi90");
}

std::string parse_det_search_mode(const std::string &value)
{
    if (value == "minima" || value == "sign")
        return value;
    throw std::runtime_error("det-search invalido; use minima ou sign");
}

BoundaryGeometryMode parse_geometry_mode(const std::string &value)
{
    if (value == "literal")
        return BoundaryGeometryMode::literal;
    if (value == "intersection" || value == "secant")
        return BoundaryGeometryMode::intersection;
    throw std::runtime_error("geometry invalida; use literal ou intersection");
}

EvenRectMatchingMode parse_even_rect_mode(const std::string &value)
{
    if (value == "paper")
        return EvenRectMatchingMode::paper;
    if (value == "square-split" || value == "square_split" || value == "square")
        return EvenRectMatchingMode::square_split;
    throw std::runtime_error("even-rect-mode invalido; use paper ou square-split");
}

void parse_args(int argc, char **argv, Params &P)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        auto next_double = [&](double &x) { if (i + 1 < argc) x = std::atof(argv[++i]); };
        auto next_int = [&](int &x) { if (i + 1 < argc) x = std::atoi(argv[++i]); };
        auto next_string = [&](std::string &x) { if (i + 1 < argc) x = argv[++i]; };

        if (arg == "--a_over_b") next_double(P.a_over_b);
        else if (arg == "--nr") next_double(P.n_r);
        else if (arg == "--N") next_int(P.N);
        else if (arg == "--Bmin") next_double(P.B_min);
        else if (arg == "--Bmax") next_double(P.B_max);
        else if (arg == "--NB") next_int(P.NB);
        else if (arg == "--Pscan") next_int(P.Pscan);
        else if (arg == "--metric") next_string(P.metric);
        else if (arg == "--det-search")
        {
            std::string value;
            next_string(value);
            P.det_search = parse_det_search_mode(value);
        }
        else if (arg == "--parity") { std::string value; next_string(value); P.parity = parse_parity(value); }
        else if (arg == "--phase") { std::string value; next_string(value); P.phase = parse_phase(value); }
        else if (arg == "--geometry") { std::string value; next_string(value); P.geometry_mode = parse_geometry_mode(value); }
        else if (arg == "--even-rect-mode") { std::string value; next_string(value); P.even_rect_mode = parse_even_rect_mode(value); }
        else if (arg == "--all-minima") P.all_minima = true;
        else if (arg == "--allow-edge-minima") P.allow_edge_minima = true;
        else if (arg == "--dump-scan") { P.dump_scan = true; next_double(P.dump_B); }
        else if (arg == "--dump-det-sign") P.dump_det_sign = true;
        else if (arg == "--rescale") P.rescale_matrix = true;
        else if (arg == "--no-rescale") P.rescale_matrix = false;
        else if (arg == "--test-bessel") P.test_bessel = true;
        else if (arg == "--family")
        {
            throw std::runtime_error("A antiga flag --family foi removida. Use --parity odd|even e --phase phi0|phi90.");
        }
    }
}