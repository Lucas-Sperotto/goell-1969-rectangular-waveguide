/*
 * cli.cpp — Implementação do parser de argumentos de linha de comando.
 *
 * Cada flag --nome corresponde a um campo da struct Params (common.hpp).
 * O parser é linear: percorre argv da esquerda para a direita e preenche
 * os campos de P na ordem em que as flags aparecem.
 *
 * Flags booleanas (sem valor):
 *   --all-minima, --allow-edge-minima, --dump-det-sign, --rescale, --no-rescale,
 *   --test-bessel, --null-vector
 *
 * Flags com valor numérico:
 *   --a_over_b, --nr, --N, --Bmin, --Bmax, --NB, --Pscan, --dump-scan
 *
 * Flags com valor de string/enum:
 *   --parity, --phase, --geometry, --even-rect-mode, --metric, --det-search
 *
 * Referência: docs/simbolos.md §"Parâmetros do CLI".
 */

#include "goell/cli.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace
{
// Garante que existe um próximo argumento para consumir como valor.
void require_value(int i, int argc, const std::string &flag)
{
    if (i + 1 >= argc)
        throw std::runtime_error("flag sem valor: " + flag);
}
} // namespace

// "odd" → harmônicos n = 1,3,5,…  [§2.1]
// "even" → harmônicos n = 0,2,4,…  [§2.1]
HarmonicParity parse_parity(const std::string &value)
{
    if (value == "odd")  return HarmonicParity::odd;
    if (value == "even") return HarmonicParity::even;
    throw std::runtime_error("parity invalida; use odd ou even");
}

// "phi0"  → Ez ∝ sin(nθ), Hz ∝ cos(nθ)  [§2.1]
// "phi90" → Ez ∝ cos(nθ), Hz ∝ sin(nθ)  [§2.1]
PhaseFamily parse_phase(const std::string &value)
{
    if (value == "phi0" || value == "0")                  return PhaseFamily::phi0;
    if (value == "phi90" || value == "pi/2" || value == "pi2") return PhaseFamily::phi90;
    throw std::runtime_error("phase invalida; use phi0 ou phi90");
}

// Valida o modo de busca: "minima" (mínimos de ln|det|) ou "sign" (mudança de sinal).
std::string parse_det_search_mode(const std::string &value)
{
    if (value == "minima" || value == "sign")
        return value;
    throw std::runtime_error("det-search invalido; use minima ou sign");
}

// "intersection" / "secant" → r = intersecção geométrica do raio θ com a borda
// "literal"                  → r calculado por escalonamento trigonométrico direto
BoundaryGeometryMode parse_geometry_mode(const std::string &value)
{
    if (value == "literal")                              return BoundaryGeometryMode::literal;
    if (value == "intersection" || value == "secant")    return BoundaryGeometryMode::intersection;
    throw std::runtime_error("geometry invalida; use literal ou intersection");
}

// "paper"        → distribuição de pontos conforme §2.2 do artigo
// "square-split" → usa lógica do guia quadrado para qualquer a/b
EvenRectMatchingMode parse_even_rect_mode(const std::string &value)
{
    if (value == "paper")                                                   return EvenRectMatchingMode::paper;
    if (value == "square-split" || value == "square_split" || value == "square") return EvenRectMatchingMode::square_split;
    throw std::runtime_error("even-rect-mode invalido; use paper ou square-split");
}

// Processa todos os argumentos argv[1..argc-1] e preenche P.
// Flags não reconhecidas são silenciosamente ignoradas (sem erro),
// exceto --family que emite mensagem de descontinuação.
void parse_args(int argc, char **argv, Params &P)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        // Lambdas auxiliares para consumir o próximo token como double / int / string.
        auto next_double = [&](double &x)
        {
            require_value(i, argc, arg);
            x = std::atof(argv[++i]);
        };
        auto next_int = [&](int &x)
        {
            require_value(i, argc, arg);
            x = std::atoi(argv[++i]);
        };
        auto next_string = [&](std::string &x)
        {
            require_value(i, argc, arg);
            x = argv[++i];
        };

        // ── Geometria ────────────────────��───────────────────────────────
        if      (arg == "--a_over_b") next_double(P.a_over_b); // a/b: razão de aspecto
        else if (arg == "--nr")       next_double(P.n_r);       // n_r = √(ε₁/ε₀)

        // ── Resolução numérica ───────────────────────────────────────────
        else if (arg == "--N")        next_int(P.N);            // harmônicos retidos
        else if (arg == "--Bmin")     next_double(P.B_min);     // B mínimo  [Eq. 6]
        else if (arg == "--Bmax")     next_double(P.B_max);     // B máximo  [Eq. 6]
        else if (arg == "--NB")       next_int(P.NB);           // passos em B
        else if (arg == "--Pscan")    next_int(P.Pscan);        // pontos na varredura de P'

        // ── Métrica e busca ──────────────────────────────────────────────
        else if (arg == "--metric")   next_string(P.metric);    // "det" ou "sv"
        else if (arg == "--det-search")
        {
            std::string value;
            next_string(value);
            P.det_search = parse_det_search_mode(value);
        }

        // ── Família de modos ──────────────────────────────��──────────────
        else if (arg == "--parity")
        {
            std::string value;
            next_string(value);
            P.parity = parse_parity(value);
        }
        else if (arg == "--phase")
        {
            std::string value;
            next_string(value);
            P.phase = parse_phase(value);
        }

        // ── Opções de geometria da fronteira ─────────────────────────────
        else if (arg == "--geometry")
        {
            std::string value;
            next_string(value);
            P.geometry_mode = parse_geometry_mode(value);
        }
        else if (arg == "--even-rect-mode")
        {
            std::string value;
            next_string(value);
            P.even_rect_mode = parse_even_rect_mode(value);
        }

        // ── Flags booleanas ──────────────────────────────────────────────
        else if (arg == "--all-minima")          P.all_minima = true;
        else if (arg == "--allow-edge-minima")   P.allow_edge_minima = true;
        else if (arg == "--rescale")             P.rescale_matrix = true;
        else if (arg == "--no-rescale")          P.rescale_matrix = false;
        else if (arg == "--test-bessel")         P.test_bessel = true;
        else if (arg == "--null-vector")         P.null_vector = true; // [F3.1]
        else if (arg == "--field-map")           P.field_map = true;   // [F3.2]
        else if (arg == "--field-B")             next_double(P.field_B);
        else if (arg == "--field-P")             next_double(P.field_Pprime);
        else if (arg == "--field-nx")            next_int(P.field_nx);
        else if (arg == "--field-ny")            next_int(P.field_ny);
        else if (arg == "--field-margin")        next_double(P.field_margin);

        // ── Modos de diagnóstico ──────────────────────────────��──────────
        else if (arg == "--dump-scan")
        {
            P.dump_scan = true;
            next_double(P.dump_B); // valor de B para o qual fazer a varredura
        }
        else if (arg == "--dump-det-sign")       P.dump_det_sign = true;

        // ── Flag descontinuada ───────────────────────────────────────────
        else if (arg == "--family")
        {
            throw std::runtime_error(
                "A antiga flag --family foi removida. Use --parity odd|even e --phase phi0|phi90.");
        }
    }
}
