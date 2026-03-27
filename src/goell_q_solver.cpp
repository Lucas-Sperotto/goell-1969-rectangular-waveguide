// goell_q_solver.cpp
//
// Reescrita "paper-faithful" do solver CHM de Goell (1969).
//
// A ideia aqui e seguir diretamente as equacoes do artigo:
// - expansoes longitudinais: eqs. (1a)-(1d)
// - campos transversos:      eqs. (3a)-(3d)
// - campo tangencial:        eqs. (4a)-(4b)
// - simetria / fases:        Sec. 2.1
// - pontos de matching:      Sec. 2.2
// - elementos de matriz:     eqs. (6a)-(6d) e (7a)-(7l)
// - matriz global Q:         eq.  (18)
// - criterio modal:          eq.  (19), det(Q)=0
//
// Duas observacoes cruciais do paper, agora explicitadas no codigo:
// 1) "odd/even" no artigo e paridade dos harmônicos n, nao "familia completa".
// 2) alem da paridade, existe a familia de fase da Sec. 2.1:
//      phi = 0     -> S = sin(n*theta),   C = cos(n*theta)
//      phi = pi/2  -> S = cos(n*theta),   C = -sin(n*theta)
//
// Portanto o problema possui quatro classes:
//   (odd harmonics, phi=0), (odd harmonics, phi=pi/2),
//   (even harmonics, phi=0), (even harmonics, phi=pi/2).
//
// Build:
//   g++ -O3 -std=c++17 -I /usr/include/eigen3 src/goell_q_solver.cpp -o build/goell_q_solver
//
// Exemplo:
//   ./build/goell_q_solver --parity odd --phase phi0 --a_over_b 1 --nr 1.01 \
//       --N 5 --Bmin 0.1 --Bmax 4 --NB 40 --Pscan 160 --metric det > out.csv

#include <bits/stdc++.h>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

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

enum class RowKind
{
    ez_long,
    hz_long,
    et_tan,
    ht_tan,
};

struct Params
{
    double a_over_b = 1.0; // razao de aspecto a/b (paper usa a e b como dimensoes completas)
    // Valor usado na implementacao para montar epsilon_r = n_r^2.
    // Mantemos este parametro separado da discussao de notacao do paper para
    // nao misturar a leitura do scan com a normalizacao numerica do solver.
    double n_r = 1.01;

    int N = 5;             // numero de harmonicos da classe escolhida
    // Eq. (16): o eixo horizontal do paper e B_paper = 2b/lambda0 * sqrt(n_r^2 - 1).
    // Como lambda0 = 2π/k0, isso implica k0 b sqrt(n_r^2 - 1) = pi * B_paper.
    double B_min = 0.1;
    double B_max = 4.0;
    int NB = 40;
    int Pscan = 160;       // varredura inicial em rho^2 = P' (eq. 11)

    HarmonicParity parity = HarmonicParity::odd;
    PhaseFamily phase = PhaseFamily::phi0;
    BoundaryGeometryMode geometry_mode = BoundaryGeometryMode::literal;

    // "det" segue a eq. (19); "sv" e apenas um auxiliar numerico.
    string metric = "det";
    // Para metric=det:
    // - "minima" reproduz o fluxo exploratorio atual, baseado em vales de log|det|
    // - "sign" procura raizes por mudanca de sinal de det(Q), mais proximo da Sec. 2.7.1
    string det_search = "minima";

    // Exporta todos os minimos locais refinados em cada B.
    // branch_id aqui e so o indice local do minimo em cada B.
    bool all_minima = true;

    // O artigo procura raizes dentro de (0,1). Por padrao, nao tratamos a
    // primeira e a ultima amostra da varredura como minimos validos, porque
    // isso tende a introduzir ramos espurios colados em Pprime ~ 0 e Pprime ~ 1.
    bool allow_edge_minima = false;

    // O paper comenta em p. 2144 que multiplicar linhas/colunas por fatores positivos
    // nao desloca os zeros do determinante. Usamos um reescalonamento bruto para ajudar
    // a condicao numerica sem alterar os zeros de det(Q).
    bool rescale_matrix = true;

    // Modo de diagnostico: exporta a varredura bruta de P' para um unico B.
    bool dump_scan = false;
    double dump_B = 0.0;
    bool dump_det_sign = false;
};

struct ColumnLayout
{
    vector<int> ez_orders; // ordens n dos coeficientes a_n e c_n
    vector<int> hz_orders; // ordens n dos coeficientes b_n e d_n

    int offset_A = 0;
    int offset_B = 0;
    int offset_C = 0;
    int offset_D = 0;
    int ncols = 0;
};

struct BoundaryPoint
{
    double theta = 0.0;
    double r = 0.0; // r_m normalizado por b
    double R = 0.0; // projecao tangencial da componente radial, eqs. (4) e definicoes sob (7)
    double T = 0.0; // projecao tangencial da componente azimutal, idem
};

struct Sample
{
    double B = 0.0;
    double Pprime = 0.0; // no paper, rho^2 da eq. (11)
    double merit = 0.0;
};

struct DeterminantInfo
{
    double logabs = -INFINITY;
    int sign = 0;
};

struct DetSample
{
    double B = 0.0;
    double Pprime = 0.0;
    double merit = 0.0;
    int sign = 0;
};

inline double Jn(int n, double x) { return std::cyl_bessel_j(n, x); }
inline double Kn(int n, double x) { return std::cyl_bessel_k(n, x); }

inline double Jn_prime(int n, double x)
{
    if (n == 0)
        return -std::cyl_bessel_j(1, x);
    return 0.5 * (std::cyl_bessel_j(n - 1, x) - std::cyl_bessel_j(n + 1, x));
}

inline double Kn_prime(int n, double x)
{
    if (n == 0)
        return -std::cyl_bessel_k(1, x);
    return -0.5 * (std::cyl_bessel_k(n - 1, x) + std::cyl_bessel_k(n + 1, x));
}

static string parity_name(HarmonicParity parity)
{
    return (parity == HarmonicParity::odd) ? "odd" : "even";
}

static string phase_name(PhaseFamily phase)
{
    return (phase == PhaseFamily::phi0) ? "phi0" : "phi90";
}

static string geometry_name(BoundaryGeometryMode mode)
{
    return (mode == BoundaryGeometryMode::literal) ? "literal" : "intersection";
}

static vector<int> odd_orders(int N)
{
    vector<int> out;
    out.reserve(N);
    for (int k = 0; k < N; ++k)
        out.push_back(2 * k + 1);
    return out;
}

static vector<int> even_orders(int N)
{
    vector<int> out;
    out.reserve(N);
    for (int k = 0; k < N; ++k)
        out.push_back(2 * k);
    return out;
}

static vector<int> drop_zero(const vector<int> &orders)
{
    vector<int> out;
    for (int n : orders)
        if (n != 0)
            out.push_back(n);
    return out;
}

// Sec. 2.1:
// - phi=0    -> S = sin(n theta),   C = cos(n theta)
// - phi=pi/2 -> S = cos(n theta),   C = -sin(n theta)
static double S_term(int n, double theta, PhaseFamily phase)
{
    return (phase == PhaseFamily::phi0) ? sin(n * theta) : cos(n * theta);
}

static double C_term(int n, double theta, PhaseFamily phase)
{
    return (phase == PhaseFamily::phi0) ? cos(n * theta) : -sin(n * theta);
}

static ColumnLayout build_layout(const Params &P)
{
    ColumnLayout L;

    if (P.parity == HarmonicParity::odd)
    {
        L.ez_orders = odd_orders(P.N);
        L.hz_orders = L.ez_orders;
    }
    else
    {
        const auto all_even = even_orders(P.N);

        // Sec. 2.2: no caso "even harmonics", uma das familias pode carregar o n=0
        // no bloco Ez e a outra no bloco Hz, resultando em 4N-2 coeficientes.
        if (P.phase == PhaseFamily::phi0)
        {
            // phi=0  -> S=sin(n theta). O termo n=0 em Ez desaparece identicamente.
            L.ez_orders = drop_zero(all_even);
            L.hz_orders = all_even;
        }
        else
        {
            // phi=pi/2 -> S=cos(n theta). O termo n=0 em Hz desaparece identicamente
            // pois C = -sin(0)=0.
            L.ez_orders = all_even;
            L.hz_orders = drop_zero(all_even);
        }
    }

    const int nA = static_cast<int>(L.ez_orders.size());
    const int nB = static_cast<int>(L.hz_orders.size());

    L.offset_A = 0;
    L.offset_B = L.offset_A + nA;
    L.offset_C = L.offset_B + nB;
    L.offset_D = L.offset_C + nA;
    L.ncols = L.offset_D + nB;

    return L;
}

// Geometria da Sec. 2 e definicoes abaixo das eqs. (7):
// - usamos a e b como dimensoes completas do paper
// - normalizamos por b, logo o retangulo fica em
//     x in [-a/(2b), +a/(2b)] = [-a_over_b/2, +a_over_b/2]
//     y in [-1/2, +1/2]
//
// geometry_mode:
// - literal: segue literalmente a leitura consolidada nos .md
// - intersection: usa a intersecao geometrica da reta radial com a fronteira
//   do retangulo, isto e, sec/csc. Este modo existe para validacao numerica,
//   pois ele parece mais consistente com a geometria da Fig. 2.
static BoundaryPoint boundary_point(double theta, double a_over_b, BoundaryGeometryMode geometry_mode)
{
    const double theta_c = atan(1.0 / a_over_b);

    BoundaryPoint bp;
    bp.theta = theta;

    if (theta < theta_c)
    {
        if (geometry_mode == BoundaryGeometryMode::literal)
        {
            // Leitura literal consolidada nos .md:
            //   r_m = (a/2) cos(theta_m)
            bp.r = (0.5 * a_over_b) * cos(theta);
        }
        else
        {
            // Intersecao com o lado vertical x = a/2:
            //   r_m cos(theta_m) = a/2  =>  r_m = (a/2) sec(theta_m)
            bp.r = (0.5 * a_over_b) / cos(theta);
        }
        bp.R = sin(theta);
        bp.T = cos(theta);
    }
    else
    {
        if (geometry_mode == BoundaryGeometryMode::literal)
        {
            // Leitura literal consolidada nos .md:
            //   r_m = (b/2) sin(theta_m)
            bp.r = 0.5 * sin(theta);
        }
        else
        {
            // Intersecao com o lado horizontal y = b/2:
            //   r_m sin(theta_m) = b/2  =>  r_m = (b/2) csc(theta_m)
            bp.r = 0.5 / sin(theta);
        }
        bp.R = -cos(theta);
        bp.T = sin(theta);
    }

    return bp;
}

static vector<double> odd_case_thetas(int N)
{
    vector<double> thetas;
    thetas.reserve(N);
    for (int m = 1; m <= N; ++m)
        thetas.push_back((m - 0.5) * PI / (2.0 * N));
    return thetas;
}

static vector<double> even_symmetry_thetas(int N)
{
    vector<double> thetas;
    thetas.reserve(N);
    for (int m = 1; m <= N; ++m)
        thetas.push_back((m - 0.5) * PI / (2.0 * N));
    return thetas;
}

static vector<double> odd_symmetry_thetas(int N)
{
    vector<double> thetas;
    if (N <= 1)
        return thetas;
    thetas.reserve(N - 1);
    // Esta forma e equivalente a
    //   theta_m = (m - N - 1/2) pi / [2 (N - 1)], m = N+1, ..., 2N-1
    // da Sec. 2.2, apenas com o indice recontado para 1, ..., N-1.
    for (int m = 1; m <= N - 1; ++m)
        thetas.push_back((m - 0.5) * PI / (2.0 * (N - 1)));
    return thetas;
}

static vector<double> omit_first_last(const vector<double> &thetas)
{
    if (thetas.size() <= 2)
        return {};
    return vector<double>(thetas.begin() + 1, thetas.end() - 1);
}

// Para "even harmonics", a Sec. 2.2 usa conjuntos diferentes de pontos para
// componentes pares e impares em torno do eixo y.
static bool uses_even_symmetry_points(RowKind row_kind, PhaseFamily phase)
{
    if (phase == PhaseFamily::phi0)
    {
        // n par e phi=0:
        //   Ez  ~ S  -> impar sobre alpha=0
        //   Hz  ~ C  -> par
        //   Et       -> par
        //   Ht       -> impar
        return row_kind == RowKind::hz_long || row_kind == RowKind::et_tan;
    }

    // n par e phi=pi/2:
    //   Ez  ~ S  -> par
    //   Hz  ~ C  -> impar
    //   Et       -> impar
    //   Ht       -> par
    return row_kind == RowKind::ez_long || row_kind == RowKind::ht_tan;
}

static RowKind odd_z_row_kind(PhaseFamily phase)
{
    // Sec. 2.1:
    // - phi=0    -> Ez ~ sin(n theta), logo Ez e o componente-z "impar"
    // - phi=pi/2 -> Hz ~ -sin(n theta), logo Hz e o componente-z "impar"
    return (phase == PhaseFamily::phi0) ? RowKind::ez_long : RowKind::hz_long;
}

static double safe_positive(double x)
{
    return (x < EPS) ? EPS : x;
}

static void row_rescale(MatrixXd &Q)
{
    for (int i = 0; i < Q.rows(); ++i)
    {
        double max_abs = 0.0;
        for (int j = 0; j < Q.cols(); ++j)
            max_abs = max(max_abs, fabs(Q(i, j)));
        if (max_abs > 0.0)
            Q.row(i) /= max_abs;
    }
}

static void col_rescale(MatrixXd &Q)
{
    for (int j = 0; j < Q.cols(); ++j)
    {
        double max_abs = 0.0;
        for (int i = 0; i < Q.rows(); ++i)
            max_abs = max(max_abs, fabs(Q(i, j)));
        if (max_abs > 0.0)
            Q.col(j) /= max_abs;
    }
}

static void fill_ez_columns(
    MatrixXd &Q,
    int row,
    RowKind row_kind,
    const ColumnLayout &L,
    const Params &P,
    const BoundaryPoint &bp,
    double kz_over_k0,
    double h_scaled,
    double p_scaled)
{
    const double eps_r = P.n_r * P.n_r;

    for (int i = 0; i < static_cast<int>(L.ez_orders.size()); ++i)
    {
        const int n = L.ez_orders[i];
        const double S = S_term(n, bp.theta, P.phase);
        const double C = C_term(n, bp.theta, P.phase);

        // O solver usa r_hat = r_m / b, h_hat = h b e p_hat = p b.
        // Assim, os argumentos das funcoes especiais continuam corretos:
        //   h r_m = h_hat * r_hat
        //   p r_m = p_hat * r_hat
        const double hr = safe_positive(h_scaled * bp.r);
        const double pr = safe_positive(p_scaled * bp.r);

        const double J0 = Jn(n, hr);
        const double K0 = Kn(n, pr);

        // Nas notas em docs/goell_02_matrix_and_normalization.md:
        //   bar J' = J'_n(h r_m) / h
        //   bar K' = K'_n(p r_m) / p
        //   bar J  = n J_n(h r_m) / (h^2 r_m)
        //   bar K  = n K_n(p r_m) / (p^2 r_m)
        //
        // Em coordenadas normalizadas por b, essas grandezas ficam proporcionais
        // as expressoes abaixo. Fatores globais comuns sao absorvidos pelo
        // reescalonamento das linhas/colunas sem mover os zeros de det(Q).
        const double JprimeRaw = Jn_prime(n, hr);
        const double JbarPrime = JprimeRaw / safe_positive(h_scaled);
        const double KbarPrime = Kn_prime(n, pr) / safe_positive(p_scaled);
        const double Jbar = (n == 0) ? 0.0 : (n * J0) / (safe_positive(h_scaled * h_scaled) * bp.r);
        const double Kbar = (n == 0) ? 0.0 : (n * K0) / (safe_positive(p_scaled * p_scaled) * bp.r);

        // Eqs. (7a), (7b), (7e), (7g), (7i), (7k)
        const double eLA = J0 * S;
        const double eLC = K0 * S;

        const double eTA = -kz_over_k0 * (JbarPrime * S * bp.R + Jbar * C * bp.T);
        const double eTC = +kz_over_k0 * (KbarPrime * S * bp.R + Kbar * C * bp.T);

        // Aqui trabalhamos com as linhas ja normalizadas por k0 e com Z0 fixado em 1,
        // exatamente como permitido pela observacao da p. 2144.
        // A documentacao revisada do paper manteve em (7i) o termo J' sem barra.
        // Por isso este bloco usa o derivado "cru" de J_n no argumento h r_m,
        // enquanto os demais blocos tangenciais seguem as formas escaladas.
        const double hTA = +eps_r * (Jbar * C * bp.R - JprimeRaw * S * bp.T);
        const double hTC = -(Kbar * C * bp.R - KbarPrime * S * bp.T);

        const int colA = L.offset_A + i;
        const int colC = L.offset_C + i;

        if (row_kind == RowKind::ez_long)
        {
            Q(row, colA) = eLA;
            Q(row, colC) = -eLC;
        }
        else if (row_kind == RowKind::et_tan)
        {
            Q(row, colA) = eTA;
            Q(row, colC) = -eTC;
        }
        else if (row_kind == RowKind::ht_tan)
        {
            Q(row, colA) = hTA;
            Q(row, colC) = -hTC;
        }
    }
}

static void fill_hz_columns(
    MatrixXd &Q,
    int row,
    RowKind row_kind,
    const ColumnLayout &L,
    const Params &P,
    const BoundaryPoint &bp,
    double kz_over_k0,
    double h_scaled,
    double p_scaled)
{
    for (int i = 0; i < static_cast<int>(L.hz_orders.size()); ++i)
    {
        const int n = L.hz_orders[i];
        const double S = S_term(n, bp.theta, P.phase);
        const double C = C_term(n, bp.theta, P.phase);

        const double hr = safe_positive(h_scaled * bp.r);
        const double pr = safe_positive(p_scaled * bp.r);

        const double J0 = Jn(n, hr);
        const double K0 = Kn(n, pr);

        const double JbarPrime = Jn_prime(n, hr) / safe_positive(h_scaled);
        const double KbarPrime = Kn_prime(n, pr) / safe_positive(p_scaled);
        const double Jbar = (n == 0) ? 0.0 : (n * J0) / (safe_positive(h_scaled * h_scaled) * bp.r);
        const double Kbar = (n == 0) ? 0.0 : (n * K0) / (safe_positive(p_scaled * p_scaled) * bp.r);

        // Eqs. (7c), (7d), (7f), (7h), (7j), (7l)
        const double hLB = J0 * C;
        const double hLD = K0 * C;

        // De novo, os fatores k0 e Z0 foram absorvidos pela normalizacao global.
        const double eTB = +(Jbar * S * bp.R + JbarPrime * C * bp.T);
        const double eTD = -(Kbar * S * bp.R + KbarPrime * C * bp.T);

        const double hTB = -kz_over_k0 * (JbarPrime * C * bp.R - Jbar * S * bp.T);
        const double hTD = +kz_over_k0 * (KbarPrime * C * bp.R - Kbar * S * bp.T);

        const int colB = L.offset_B + i;
        const int colD = L.offset_D + i;

        if (row_kind == RowKind::hz_long)
        {
            Q(row, colB) = hLB;
            Q(row, colD) = -hLD;
        }
        else if (row_kind == RowKind::et_tan)
        {
            Q(row, colB) = eTB;
            Q(row, colD) = -eTD;
        }
        else if (row_kind == RowKind::ht_tan)
        {
            Q(row, colB) = hTB;
            Q(row, colD) = -hTD;
        }
    }
}

static void append_row(
    MatrixXd &Q,
    int row,
    RowKind row_kind,
    const ColumnLayout &L,
    const Params &P,
    const BoundaryPoint &bp,
    double kz_over_k0,
    double h_scaled,
    double p_scaled)
{
    fill_ez_columns(Q, row, row_kind, L, P, bp, kz_over_k0, h_scaled, p_scaled);
    fill_hz_columns(Q, row, row_kind, L, P, bp, kz_over_k0, h_scaled, p_scaled);
}

static MatrixXd assemble_Q(const Params &P, double B, double Pprime)
{
    const ColumnLayout L = build_layout(P);

    // Eq. (11): Pprime aqui representa a variavel vertical normalizada.
    // Eq. (16): a entrada B corresponde ao eixo horizontal do paper, B_paper.
    // Como lambda0 = 2 pi / k0, temos:
    //   k0 b sqrt(n_r^2 - 1) = pi * B_paper.
    // Com r_hat = r_m / b, os argumentos das funcoes especiais ficam:
    //   h r_m = (pi B_paper) r_hat sqrt(1 - Pprime)
    //   p r_m = (pi B_paper) r_hat sqrt(Pprime)
    const double eps_r = P.n_r * P.n_r;
    const double kz_over_k0 = sqrt(1.0 + (eps_r - 1.0) * Pprime);

    const double Bsafe = safe_positive(B);
    const double radial_scale = PI * Bsafe;
    const double h_scaled = radial_scale * sqrt(safe_positive(1.0 - Pprime));
    const double p_scaled = radial_scale * sqrt(safe_positive(Pprime));

    MatrixXd Q;

    if (P.parity == HarmonicParity::odd)
    {
        // Sec. 2.2: odd harmonics usam um unico conjunto de N pontos.
        const auto thetas = odd_case_thetas(P.N);
        Q = MatrixXd::Zero(4 * P.N, L.ncols);

        int row = 0;
        for (double theta : thetas)
        {
            const auto bp = boundary_point(theta, P.a_over_b, P.geometry_mode);
            append_row(Q, row++, RowKind::ez_long, L, P, bp, kz_over_k0, h_scaled, p_scaled); // eq. (6a)
            append_row(Q, row++, RowKind::hz_long, L, P, bp, kz_over_k0, h_scaled, p_scaled); // eq. (6b)
            append_row(Q, row++, RowKind::et_tan, L, P, bp, kz_over_k0, h_scaled, p_scaled);  // eq. (6c)
            append_row(Q, row++, RowKind::ht_tan, L, P, bp, kz_over_k0, h_scaled, p_scaled);  // eq. (6d)
        }
    }
    else
    {
        // Sec. 2.2:
        // - para a/b = 1:
        //     componentes "pares" em y usam N pontos
        //     componentes "impares" em y usam N-1 pontos
        // - para a/b != 1:
        //     todos usam a primeira formula (N pontos),
        //     exceto o componente-z impar, que omite o primeiro e o ultimo ponto.
        const auto theta_full = even_symmetry_thetas(P.N);
        const auto theta_odd_square = odd_symmetry_thetas(P.N);
        const auto theta_odd_rect = omit_first_last(theta_full);
        const bool is_square = fabs(P.a_over_b - 1.0) <= 1e-12;

        // 2N linhas para a classe "par" e 2(N-1) para a classe "impar" => 4N-2.
        Q = MatrixXd::Zero(4 * P.N - 2, L.ncols);

        int row = 0;
        for (RowKind kind : {RowKind::ez_long, RowKind::hz_long, RowKind::et_tan, RowKind::ht_tan})
        {
            const vector<double> *thetas = nullptr;
            if (is_square)
            {
                const bool use_even_points = uses_even_symmetry_points(kind, P.phase);
                thetas = use_even_points ? &theta_full : &theta_odd_square;
            }
            else
            {
                thetas = (kind == odd_z_row_kind(P.phase)) ? &theta_odd_rect : &theta_full;
            }

            for (double theta : *thetas)
            {
                const auto bp = boundary_point(theta, P.a_over_b, P.geometry_mode);
                append_row(Q, row++, kind, L, P, bp, kz_over_k0, h_scaled, p_scaled);
            }
        }
    }

    if (P.rescale_matrix)
    {
        row_rescale(Q);
        col_rescale(Q);
    }

    return Q;
}

static double logabs_det(const MatrixXd &Q)
{
    FullPivLU<MatrixXd> lu(Q);
    const auto &LU = lu.matrixLU();
    double s = 0.0;

    for (int i = 0; i < LU.rows(); ++i)
    {
        const double di = fabs(LU(i, i));
        if (!(di > 0.0) || !isfinite(di))
            return -INFINITY;
        s += log(di);
    }

    return s;
}

static DeterminantInfo determinant_info(const MatrixXd &Q)
{
    FullPivLU<MatrixXd> lu(Q);
    const auto &LU = lu.matrixLU();

    DeterminantInfo info;
    info.logabs = 0.0;

    // det(Q) = det(P)^-1 det(LU) det(Qperm)^-1.
    // Como as permutacoes tem determinante +-1, basta acompanhar o sinal.
    int sign = static_cast<int>(std::round(lu.permutationP().determinant() * lu.permutationQ().determinant()));
    if (sign == 0)
        sign = 1;

    for (int i = 0; i < LU.rows(); ++i)
    {
        const double di = LU(i, i);
        if (!isfinite(di) || fabs(di) <= 0.0)
        {
            info.logabs = -INFINITY;
            info.sign = 0;
            return info;
        }

        info.logabs += log(fabs(di));
        sign *= (di < 0.0) ? -1 : 1;
    }

    info.sign = sign;
    return info;
}

static double log10_sigma_rel(const MatrixXd &Q)
{
    JacobiSVD<MatrixXd> svd(Q, ComputeThinU | ComputeThinV);
    const auto s = svd.singularValues();
    if (s.size() == 0 || s(0) <= 0.0)
        return 0.0;
    return log10(s.tail(1)(0) / s(0));
}

static double merit_value(const Params &P, double B, double Pprime)
{
    const auto Q = assemble_Q(P, B, Pprime);
    return (P.metric == "sv") ? log10_sigma_rel(Q) : logabs_det(Q);
}

static DeterminantInfo determinant_value(const Params &P, double B, double Pprime)
{
    return determinant_info(assemble_Q(P, B, Pprime));
}

static vector<Sample> scan_P(const Params &P, double B)
{
    vector<Sample> out;
    out.reserve(P.Pscan);

    for (int i = 0; i < P.Pscan; ++i)
    {
        // Eq. (19): o paper testa valores distribuidos em (0,1) para localizar raizes.
        // Usamos pontos estritamente interiores para evitar "minimos" artificiais nas bordas.
        const double Pprime = (i + 1.0) / double(P.Pscan + 1);
        out.push_back({B, Pprime, merit_value(P, B, Pprime)});
    }

    return out;
}

static vector<DetSample> scan_P_det(const Params &P, double B)
{
    vector<DetSample> out;
    out.reserve(P.Pscan);

    for (int i = 0; i < P.Pscan; ++i)
    {
        const double Pprime = (i + 1.0) / double(P.Pscan + 1);
        const auto info = determinant_value(P, B, Pprime);
        out.push_back({B, Pprime, info.logabs, info.sign});
    }

    return out;
}

static vector<Sample> local_minima(const vector<Sample> &samples)
{
    vector<Sample> mins;
    if (samples.size() < 2)
        return mins;

    for (size_t i = 1; i + 1 < samples.size(); ++i)
    {
        if (isfinite(samples[i].merit) &&
            samples[i].merit < samples[i - 1].merit &&
            samples[i].merit < samples[i + 1].merit)
        {
            mins.push_back(samples[i]);
        }
    }

    return mins;
}

static vector<Sample> edge_minima(const vector<Sample> &samples)
{
    vector<Sample> mins;
    if (samples.size() < 2)
        return mins;

    if (isfinite(samples.front().merit) && samples.front().merit < samples[1].merit)
        mins.push_back(samples.front());

    if (isfinite(samples.back().merit) && samples.back().merit < samples[samples.size() - 2].merit)
        mins.push_back(samples.back());

    return mins;
}

static vector<Sample> sign_change_roots(const Params &P, double B, const vector<DetSample> &samples)
{
    vector<Sample> roots;
    if (samples.size() < 2)
        return roots;

    auto append_unique = [&](double pref)
    {
        const double clamped = min(1.0 - 1e-6, max(1e-6, pref));
        if (!roots.empty() && fabs(roots.back().Pprime - clamped) < 1e-5)
            return;
        const auto info = determinant_value(P, B, clamped);
        roots.push_back({B, clamped, info.logabs});
    };

    for (size_t i = 0; i + 1 < samples.size(); ++i)
    {
        const auto &left = samples[i];
        const auto &right = samples[i + 1];

        if (left.sign == 0)
        {
            append_unique(left.Pprime);
            continue;
        }

        if (left.sign != 0 && right.sign != 0 && left.sign != right.sign)
        {
            double a = left.Pprime;
            double b = right.Pprime;
            int sa = left.sign;
            int sb = right.sign;

            for (int it = 0; it < 50; ++it)
            {
                const double mid = 0.5 * (a + b);
                const auto info_mid = determinant_value(P, B, mid);

                if (info_mid.sign == 0)
                {
                    a = b = mid;
                    break;
                }

                if (sa != 0 && sa != info_mid.sign)
                {
                    b = mid;
                    sb = info_mid.sign;
                }
                else if (sb != 0 && sb != info_mid.sign)
                {
                    a = mid;
                    sa = info_mid.sign;
                }
                else
                {
                    // Se o sinal no ponto medio nao ajuda a manter um bracket limpo,
                    // paramos aqui e usamos o centro do intervalo atual.
                    break;
                }
            }

            append_unique(0.5 * (a + b));
        }
    }

    return roots;
}

static double refine_local_minimum(const Params &P, double B, double x0)
{
    auto f = [&](double x)
    {
        const double clamped = min(1.0 - 1e-6, max(1e-6, x));
        return merit_value(P, B, clamped);
    };

    const double dx = 2.0 / double(P.Pscan + 1);
    double x1 = max(1e-6, x0 - dx);
    double x2 = x0;
    double x3 = min(1.0 - 1e-6, x0 + dx);

    for (int it = 0; it < 20; ++it)
    {
        const double y1 = f(x1);
        const double y2 = f(x2);
        const double y3 = f(x3);

        const double D = (x1 - x2) * (x1 - x3) * (x2 - x3);
        if (fabs(D) < 1e-18)
            break;

        const double A = (y1 * (x2 - x3) + y2 * (x3 - x1) + y3 * (x1 - x2)) / D;
        const double Bc = (y1 * (x3 * x3 - x2 * x2) +
                           y2 * (x1 * x1 - x3 * x3) +
                           y3 * (x2 * x2 - x1 * x1)) /
                          D;

        if (fabs(A) < 1e-18)
            break;

        const double xm = -Bc / (2.0 * A);
        if (!isfinite(xm))
            break;

        const double xmid = min(x3, max(x1, xm));

        const double h = 0.6 * 0.5 * (x3 - x1);
        x1 = max(1e-6, xmid - h);
        x2 = min(1.0 - 1e-6, max(1e-6, xmid));
        x3 = min(1.0 - 1e-6, xmid + h);
    }

    return x2;
}

static HarmonicParity parse_parity(const string &value)
{
    if (value == "odd")
        return HarmonicParity::odd;
    if (value == "even")
        return HarmonicParity::even;
    throw runtime_error("parity invalida; use odd ou even");
}

static PhaseFamily parse_phase(const string &value)
{
    if (value == "phi0" || value == "0")
        return PhaseFamily::phi0;
    if (value == "phi90" || value == "pi/2" || value == "pi2")
        return PhaseFamily::phi90;
    throw runtime_error("phase invalida; use phi0 ou phi90");
}

static string parse_det_search_mode(const string &value)
{
    if (value == "minima" || value == "sign")
        return value;
    throw runtime_error("det-search invalido; use minima ou sign");
}

static BoundaryGeometryMode parse_geometry_mode(const string &value)
{
    if (value == "literal")
        return BoundaryGeometryMode::literal;
    if (value == "intersection" || value == "secant")
        return BoundaryGeometryMode::intersection;
    throw runtime_error("geometry invalida; use literal ou intersection");
}

static void parse_args(int argc, char **argv, Params &P)
{
    for (int i = 1; i < argc; ++i)
    {
        const string arg = argv[i];

        auto next_double = [&](double &x)
        {
            if (i + 1 < argc)
                x = atof(argv[++i]);
        };
        auto next_int = [&](int &x)
        {
            if (i + 1 < argc)
                x = atoi(argv[++i]);
        };
        auto next_string = [&](string &x)
        {
            if (i + 1 < argc)
                x = argv[++i];
        };

        if (arg == "--a_over_b")
            next_double(P.a_over_b);
        else if (arg == "--nr")
            next_double(P.n_r);
        else if (arg == "--N")
            next_int(P.N);
        else if (arg == "--Bmin")
            next_double(P.B_min);
        else if (arg == "--Bmax")
            next_double(P.B_max);
        else if (arg == "--NB")
            next_int(P.NB);
        else if (arg == "--Pscan")
            next_int(P.Pscan);
        else if (arg == "--metric")
            next_string(P.metric);
        else if (arg == "--det-search")
        {
            string value;
            next_string(value);
            P.det_search = parse_det_search_mode(value);
        }
        else if (arg == "--parity")
        {
            string value;
            next_string(value);
            P.parity = parse_parity(value);
        }
        else if (arg == "--phase")
        {
            string value;
            next_string(value);
            P.phase = parse_phase(value);
        }
        else if (arg == "--geometry")
        {
            string value;
            next_string(value);
            P.geometry_mode = parse_geometry_mode(value);
        }
        else if (arg == "--all-minima")
            P.all_minima = true;
        else if (arg == "--allow-edge-minima")
            P.allow_edge_minima = true;
        else if (arg == "--dump-scan")
        {
            P.dump_scan = true;
            next_double(P.dump_B);
        }
        else if (arg == "--dump-det-sign")
            P.dump_det_sign = true;
        else if (arg == "--rescale")
            P.rescale_matrix = true;
        else if (arg == "--no-rescale")
            P.rescale_matrix = false;
        else if (arg == "--family")
        {
            throw runtime_error(
                "A antiga flag --family foi removida. Use --parity odd|even e --phase phi0|phi90.");
        }
    }
}

int main(int argc, char **argv)
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Params P;
    try
    {
        parse_args(argc, argv, P);
    }
    catch (const exception &e)
    {
        cerr << "[goell_q_solver] " << e.what() << "\n";
        return 1;
    }

    cout.setf(std::ios::fixed);
    cout << setprecision(6);

    if (P.dump_scan)
    {
        if (P.dump_det_sign && P.metric == "det")
            cout << "B,Pprime,merit,det_sign,parity,phase,geometry\n";
        else
            cout << "B,Pprime,merit,parity,phase,geometry\n";

        for (const auto &sample : scan_P(P, P.dump_B))
        {
            if (P.dump_det_sign && P.metric == "det")
            {
                const auto info = determinant_info(assemble_Q(P, sample.B, sample.Pprime));
                cout << sample.B << "," << sample.Pprime << "," << sample.merit << ","
                     << info.sign << "," << parity_name(P.parity) << "," << phase_name(P.phase) << ","
                     << geometry_name(P.geometry_mode) << "\n";
            }
            else
            {
                cout << sample.B << "," << sample.Pprime << "," << sample.merit << ","
                     << parity_name(P.parity) << "," << phase_name(P.phase) << ","
                     << geometry_name(P.geometry_mode) << "\n";
            }
        }
        return 0;
    }

    cout << "branch_id,B,Pprime,merit,parity,phase,geometry\n";

    for (int iB = 0; iB <= P.NB; ++iB)
    {
        const double B = (P.NB <= 0)
                             ? P.B_min
                             : P.B_min + (P.B_max - P.B_min) * double(iB) / double(P.NB);

        // Em B=0 as eqs. (14)-(15) colapsam para hr=pr=0, o que torna a busca por
        // minimos locais numericamente degenerada. Mantemos o eixo do grafico iniciando
        // em zero, mas pulamos a avaliacao exatamente nesse ponto.
        if (fabs(B) < 1e-12)
            continue;

        vector<Sample> candidates;
        if (P.metric == "det" && P.det_search == "sign")
        {
            const auto det_scan = scan_P_det(P, B);
            candidates = sign_change_roots(P, B, det_scan);
        }
        else
        {
            auto coarse = scan_P(P, B);
            candidates = local_minima(coarse);
            if (P.allow_edge_minima)
            {
                auto e = edge_minima(coarse);
                candidates.insert(candidates.end(), e.begin(), e.end());
            }
            sort(candidates.begin(), candidates.end(), [](const Sample &a, const Sample &b)
                 { return a.Pprime < b.Pprime; });

            if (!P.all_minima && !candidates.empty())
            {
                candidates = {*min_element(candidates.begin(), candidates.end(), [](const Sample &a, const Sample &b)
                                           { return a.merit < b.merit; })};
            }
        }

        for (size_t k = 0; k < candidates.size(); ++k)
        {
            double pref = candidates[k].Pprime;
            double merit = candidates[k].merit;
            if (!(P.metric == "det" && P.det_search == "sign"))
            {
                pref = refine_local_minimum(P, B, pref);
                merit = merit_value(P, B, pref);
            }
            cout << k << "," << B << "," << pref << "," << merit << ","
                 << parity_name(P.parity) << "," << phase_name(P.phase) << ","
                 << geometry_name(P.geometry_mode) << "\n";
        }
    }

    return 0;
}
