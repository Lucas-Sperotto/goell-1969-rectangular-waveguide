/*
 * field.cpp — Avaliação dos campos eletromagnéticos transversais (F3.2).
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * DERIVAÇÃO DOS COMPONENTES DE CAMPO (artigo §2, Eqs. 1-3)
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Dado o vetor nulo v = [aₙ | bₙ | cₙ | dₙ]ᵀ de Q no ponto modal (B, P'),
 * os campos são expandidos conforme Eqs. (1a-d) e (3a-d).
 *
 * ── Componentes longitudinais (reais, fase z=0 t=0) ──────────────────────
 *
 *   Ez_in = Σₙ aₙ · Jₙ(h·r) · S(n,θ)   [Eq. 1a]
 *   Hz_in = Σₙ bₙ · Jₙ(h·r) · C(n,θ)   [Eq. 1b]
 *   (idem com Kₙ, cₙ, dₙ no exterior)
 *
 * ── Componentes transversais (fase ωt = π/2, onde Et e Ht são reais) ─────
 *
 * Em notação complexa, as Eqs. (3a-d) têm o fator i·kz/(k²−kz²).  A componente
 * i extrai um avanço de fase de π/2: tomando o campo real na fase ωt = π/2
 * (quando Ez = Hz = 0), o fator i·exp[−iπ/2] = i·(−i) = 1, tornando Et e Ht reais.
 *
 * Com k² − kz² = +h²  (interior) ou = −p² (exterior), e usando as variáveis
 * intermediárias Jr, Jth, Kr, Kth de matrix.cpp:
 *
 *   Jr  = J'ₙ(h·r) / h_scaled        K'ₙ(p·r) / p_scaled
 *   Jth = n·Jₙ(h·r) / (h_scaled²·r)  n·Kₙ(p·r) / (p_scaled²·r)
 *   S   = S_term(n,θ)  ·  C = C_term(n,θ)
 *
 * ── Interior (k=k₁, k²−kz²=h²) ──────────────────────────────────────────
 *
 *   Contribuição dos coeficientes aₙ (Ez interior):
 *     Er  += aₙ · (−kz · Jr · S)       [de (3a): kz/h² · ∂Ez/∂r]
 *     Eθ  += aₙ · (−kz · Jth · C)      [de (3b): kz/h² · (1/r)·∂Ez/∂θ]
 *     Hr  += aₙ · (+ε_r · Jth · C)     [de (3c): −k²/(μω·kz·r)·∂Ez/∂θ → ε_r·Jth·C]
 *     Hθ  += aₙ · (−ε_r · Jr · S)      [de (3d): k²/(μω·kz)·∂Ez/∂r  → −ε_r·Jr·S]
 *
 *   Contribuição dos coeficientes bₙ (Hz interior):
 *     Er  += bₙ · (+Jth · S)           [de (3a): (μω/kz)/h²·(1/r)·∂Hz/∂θ → +Jth·S]
 *     Eθ  += bₙ · (+Jr · C)            [de (3b): −(μω/kz)/h²·∂Hz/∂r  → +Jr·C]
 *     Hr  += bₙ · (−kz · Jr · C)       [de (3c): ∂Hz/∂r  → −kz·Jr·C]
 *     Hθ  += bₙ · (+kz · Jth · S)      [de (3d): (1/r)·∂Hz/∂θ → +kz·Jth·S]
 *
 * ── Exterior (k=k₀, k²−kz²=−p²) ─────────────────────────────────────────
 *
 *   A mudança de sinal em k²−kz² inverte o sinal de todos os termos de Et e Ht
 *   relativamente ao interior (como verificado nos blocos eTC, eTD, hTC, hTD de Q):
 *
 *   Contribuição dos coeficientes cₙ (Ez exterior):
 *     Er  += cₙ · (+kz · Kr · S)       [sinal oposto a interior]
 *     Eθ  += cₙ · (+kz · Kth · C)
 *     Hr  += cₙ · (−Kth · C)           [sem ε_r: k₀²/p² → 1]
 *     Hθ  += cₙ · (+Kr · S)
 *
 *   Contribuição dos coeficientes dₙ (Hz exterior):
 *     Er  += dₙ · (−Kth · S)
 *     Eθ  += dₙ · (−Kr · C)
 *     Hr  += dₙ · (+kz · Kr · C)
 *     Hθ  += dₙ · (−kz · Kth · S)
 *
 * Referência: docs/02.3_formulacao_dos_elementos_de_matriz.md  docs/simbolos.md §"Blocos de Q".
 */

#include "goell/field.hpp"

#include "goell/bessel.hpp"
#include "goell/diagnostics.hpp"
#include "goell/layout.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

using Eigen::VectorXd;

namespace
{

// Retorna true se o ponto (x,y) está dentro do núcleo retangular.
// Coordenadas normalizadas por b (b=1): núcleo = |x|≤a_over_b/2, |y|≤0.5.
bool is_inside_core(double x, double y, double a_over_b)
{
    return std::fabs(x) <= 0.5 * a_over_b + EPS && std::fabs(y) <= 0.5 + EPS;
}

// Guard contra divisão por zero idêntico ao de matrix.cpp.
double safe_denom(double x)
{
    return (x < EPS) ? EPS : x;
}

// Acumula os 6 componentes transversais (Er, Eθ, Hr, Hθ) e os 2 longitudinais
// (Ez, Hz) para o n-ésimo harmônico INTERIOR (funções Jₙ, coeficientes aₙ e bₙ).
//
// Os sinais seguem a derivação acima e são consistentes com os blocos de Q
// em matrix.cpp (eTA, eTB, hTA, hTB) [Eqs. 7e-h, 7i-l].
void accumulate_interior(
    FieldPoint &fp,
    int         n,
    double      coeff_a,   // aₙ
    double      coeff_b,   // bₙ
    double      h_scaled,
    double      r,
    double      theta,
    double      kz,        // kz_over_k0
    double      eps_r,
    PhaseFamily phase)
{
    const double hr  = h_scaled * r;
    const double J0  = Jn(n, hr);
    const double Jr  = Jn_prime(n, hr) / safe_denom(h_scaled);
    const double Jth = (n == 0) ? 0.0 : (n * J0) / (safe_denom(h_scaled * h_scaled) * r);

    const double S = S_term(n, theta, phase); // sin(nθ) ou cos(nθ) conforme a família
    const double C = C_term(n, theta, phase); // cos(nθ) ou −sin(nθ)

    // ── Campos longitudinais [Eqs. 1a, 1b] ─────────────────────────────
    fp.Ez += coeff_a * J0 * S;
    fp.Hz += coeff_b * J0 * C;

    // ── Er e Eθ [Eqs. 3a-b, contribuições de aₙ e bₙ] ─────────────────
    fp.Er     += coeff_a * (-kz * Jr * S)    + coeff_b * (+Jth * S);
    fp.Etheta += coeff_a * (-kz * Jth * C)   + coeff_b * (+Jr  * C);

    // ── Hr e Hθ [Eqs. 3c-d, contribuições de aₙ e bₙ] ─────────────────
    fp.Hr     += coeff_a * (+eps_r * Jth * C) + coeff_b * (-kz * Jr  * C);
    fp.Htheta += coeff_a * (-eps_r * Jr  * S) + coeff_b * (+kz * Jth * S);
}

// Acumula os componentes para o n-ésimo harmônico EXTERIOR (funções Kₙ, coeficientes cₙ e dₙ).
// O sinal invertido relativamente ao interior vem de k²−kz²=−p² < 0 [Eqs. 7f, 7h, 7j, 7l].
// Não há fator ε_r no exterior (k₀²/p² → 1 na normalização).
void accumulate_exterior(
    FieldPoint &fp,
    int         n,
    double      coeff_c,   // cₙ
    double      coeff_d,   // dₙ
    double      p_scaled,
    double      r,
    double      theta,
    double      kz,
    PhaseFamily phase)
{
    const double pr  = p_scaled * r;
    const double K0  = Kn(n, pr);
    const double Kr  = Kn_prime(n, pr) / safe_denom(p_scaled);
    const double Kth = (n == 0) ? 0.0 : (n * K0) / (safe_denom(p_scaled * p_scaled) * r);

    const double S = S_term(n, theta, phase);
    const double C = C_term(n, theta, phase);

    // ── Campos longitudinais [Eqs. 1c, 1d] ─────────────────────────────
    fp.Ez += coeff_c * K0 * S;
    fp.Hz += coeff_d * K0 * C;

    // ── Er e Eθ [sinais invertidos em relação ao interior] ───────────────
    fp.Er     += coeff_c * (+kz * Kr  * S)  + coeff_d * (-Kth * S);
    fp.Etheta += coeff_c * (+kz * Kth * C)  + coeff_d * (-Kr  * C);

    // ── Hr e Hθ [sem ε_r, sinais conforme Eqs. 7j, 7l] ──────────────────
    fp.Hr     += coeff_c * (-Kth * C)        + coeff_d * (+kz * Kr  * C);
    fp.Htheta += coeff_c * (+Kr  * S)        + coeff_d * (-kz * Kth * S);
}

} // namespace

// ─── eval_field_point ────────────────────────────────────────────────────────
//
// Avalia todos os 10 componentes de campo num ponto (x,y).
// Primeiro determina se o ponto é interior ou exterior ao núcleo retangular,
// depois itera sobre os harmônicos do bloco correspondente.
FieldPoint eval_field_point(
    const Params         &P,
    const ColumnLayout   &L,
    double                h_scaled,
    double                p_scaled,
    double                kz_over_k0,
    const VectorXd       &v,
    double                x,
    double                y)
{
    FieldPoint fp;
    fp.x      = x;
    fp.y      = y;
    fp.inside = is_inside_core(x, y, P.a_over_b);

    const double eps_r = P.n_r * P.n_r;

    // Converte para cilíndricas.
    const double r     = std::sqrt(x * x + y * y);
    const double theta = std::atan2(y, x);

    // Evita singularidade na origem (Jth ∝ 1/r, mas n≥1 → Jₙ(hr)/r → 0 contínuo;
    // todavia, num ponto r=0 exato, a fórmula de recorrência pode ser instável).
    if (r < 1e-9)
        return fp; // todos os campos = 0 (continuidade prevista)

    const int nA = static_cast<int>(L.ez_orders.size());
    const int nB = static_cast<int>(L.hz_orders.size());

    if (fp.inside)
    {
        // ── Interior: somar contribuições de aₙ e bₙ ──────────────────
        // Os índices de coluna seguem o ColumnLayout: A=[0,nA), B=[nA,nA+nB).
        for (int i = 0; i < nA || i < nB; ++i)
        {
            const double coeff_a = (i < nA) ? v(L.offset_A + i) : 0.0;
            const double coeff_b = (i < nB) ? v(L.offset_B + i) : 0.0;
            const int    n_a     = (i < nA) ? L.ez_orders[i] : 0;
            const int    n_b     = (i < nB) ? L.hz_orders[i] : 0;

            // ez_orders e hz_orders podem diferir em paridade even.
            // Processa separadamente por harmônico para respeitar as ordens corretas.
            if (i < nA)
                accumulate_interior(fp, n_a, coeff_a, 0.0,
                                    h_scaled, r, theta, kz_over_k0, eps_r, P.phase);
            if (i < nB)
                accumulate_interior(fp, n_b, 0.0, coeff_b,
                                    h_scaled, r, theta, kz_over_k0, eps_r, P.phase);
        }
    }
    else
    {
        // ── Exterior: somar contribuições de cₙ e dₙ ──────────────────
        for (int i = 0; i < nA || i < nB; ++i)
        {
            const double coeff_c = (i < nA) ? v(L.offset_C + i) : 0.0;
            const double coeff_d = (i < nB) ? v(L.offset_D + i) : 0.0;
            const int    n_c     = (i < nA) ? L.ez_orders[i] : 0;
            const int    n_d     = (i < nB) ? L.hz_orders[i] : 0;

            if (i < nA)
                accumulate_exterior(fp, n_c, coeff_c, 0.0,
                                    p_scaled, r, theta, kz_over_k0, P.phase);
            if (i < nB)
                accumulate_exterior(fp, n_d, 0.0, coeff_d,
                                    p_scaled, r, theta, kz_over_k0, P.phase);
        }
    }

    // ── Converte (Er, Eθ) e (Hr, Hθ) para Cartesianos ─────────────────
    const double cos_th = std::cos(theta);
    const double sin_th = std::sin(theta);

    fp.Ex = fp.Er * cos_th - fp.Etheta * sin_th;
    fp.Ey = fp.Er * sin_th + fp.Etheta * cos_th;
    fp.Hx = fp.Hr * cos_th - fp.Htheta * sin_th;
    fp.Hy = fp.Hr * sin_th + fp.Htheta * cos_th;

    return fp;
}

// ─── compute_field_grid ──────────────────────────────────────────────────────
//
// Monta a grade retangular de pontos de campo para o modo (B, Pprime).
//
// Extensão da grade (em unidades de b):
//   x ∈ [−margin·(a/2b),  +margin·(a/2b)]   (a_over_b/2 = semi-largura do núcleo)
//   y ∈ [−margin·(1/2),   +margin·(1/2)]      (0.5 = semi-altura do núcleo)
//
// Os pontos são normalizados pelo maior |Ez ou Hz| no interior para que o pico
// da componente longitudinal dominante seja ±1.  Essa normalização preserva
// as amplitudes relativas entre Ez, Hz, Et e Ht.
std::vector<FieldPoint> compute_field_grid(
    const Params &P,
    double        B,
    double        Pprime)
{
    const ColumnLayout L  = build_layout(P);
    const double eps_r    = P.n_r * P.n_r;
    const double kz       = std::sqrt(1.0 + (eps_r - 1.0) * Pprime);
    const double radscale = PI * std::max(0.0, B);
    const double h_sc     = radscale * std::sqrt(std::max(0.0, 1.0 - Pprime));
    const double p_sc     = radscale * std::sqrt(std::max(0.0, Pprime));

    // Obtém o vetor nulo v = [aₙ | bₙ | cₙ | dₙ]ᵀ via SVD de Q.
    const VectorXd v = compute_null_vector(P, B, Pprime);

    // Extensão da grade: margin × semi-largura do núcleo.
    const double xmax = P.field_margin * 0.5 * P.a_over_b;
    const double ymax = P.field_margin * 0.5;

    const int nx = std::max(2, P.field_nx);
    const int ny = std::max(2, P.field_ny);

    const double dx = 2.0 * xmax / static_cast<double>(nx - 1);
    const double dy = 2.0 * ymax / static_cast<double>(ny - 1);

    std::vector<FieldPoint> grid;
    grid.reserve(static_cast<std::size_t>(nx * ny));

    for (int iy = 0; iy < ny; ++iy)
    {
        const double y = -ymax + iy * dy;
        for (int ix = 0; ix < nx; ++ix)
        {
            const double x = -xmax + ix * dx;
            grid.push_back(eval_field_point(P, L, h_sc, p_sc, kz, v, x, y));
        }
    }

    // ── Normalização pelo pico global de Ez/Hz em toda a grade ───────────
    //
    // Estratégia em cascata para cobrir todos os regimes:
    //   1. Pico de |Ez| ou |Hz| em TODA a grade  (interior + exterior).
    //      Perto do cutoff (P'→0 ou P'→1), os modos têm Kₙ(pr) grandes para
    //      harmônicos altos (n=4,5): o campo exterior pode ser ordens de magnitude
    //      maior que o interior.  Normalizar pelo pico global garante saída
    //      limitada em [-1, 1] e colorbar significativa.
    //   2. Pico de |Et| ou |Ht| em toda a grade  (modo puramente transversal,
    //      ex. Ez ≈ Hz ≈ 0 para um modo TE/TM degenerado).
    // O limiar 1e-6 distingue "pico genuíno" de ruído numérico.
    double peak = 0.0;

    for (const auto &fp : grid)  // estratégia 1: longitudinais em todo o grid
    {
        peak = std::max(peak, std::fabs(fp.Ez));
        peak = std::max(peak, std::fabs(fp.Hz));
    }

    if (peak < 1e-6)             // estratégia 2: campo transversal total
        for (const auto &fp : grid)
        {
            peak = std::max(peak, std::hypot(fp.Er, fp.Etheta));
            peak = std::max(peak, std::hypot(fp.Hr, fp.Htheta));
        }

    if (peak > 0.0)
    {
        for (auto &fp : grid)
        {
            fp.Ez     /= peak;  fp.Hz     /= peak;
            fp.Er     /= peak;  fp.Etheta /= peak;
            fp.Hr     /= peak;  fp.Htheta /= peak;
            fp.Ex     /= peak;  fp.Ey     /= peak;
            fp.Hx     /= peak;  fp.Hy     /= peak;
        }
    }

    return grid;
}
