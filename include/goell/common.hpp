/*
 * common.hpp — Tipos globais e struct Params do solver Goell CHM.
 *
 * O guia de onda estudado no artigo (§2) é um núcleo retangular com
 * constante dielétrica ε₁ imerso em meio infinito ε₀, ambos isotrópicos
 * com permeabilidade μ₀.  O campo é expandido em harmônicos circulares nas
 * coordenadas cilíndricas (r, θ) — o chamado Método dos Harmônicos Circulares
 * (CHM).  Todos os módulos do solver dependem deste cabeçalho.
 *
 * Referência: docs/02_derivacao_das_equacoes.md  §2 do artigo Goell (1969).
 */

#ifndef GOELL_COMMON_HPP
#define GOELL_COMMON_HPP

#include <string>

// Constantes numéricas de uso global.
static constexpr double PI  = 3.14159265358979323846;
static constexpr double EPS = 1e-12; // denominador mínimo para evitar divisão por zero

// ─── Paridade dos harmônicos ─────────────────────────────────────────────────
//
// Goell §2.1 mostra que a simetria da seção transversal retangular reduz a
// expansão a dois sub-conjuntos de ordens n:
//   odd  → n = 1, 3, 5, …  (harmônicos ímpares)
//   even → n = 0, 2, 4, …  (harmônicos pares)
// Modos com paridade ímpar e par são independentes; cada um gera uma
// família de curvas de dispersão.
enum class HarmonicParity
{
    odd,   // harmônicos ímpares: n = 2k+1, k = 0,1,…,N-1
    even,  // harmônicos pares:   n = 2k,   k = 0,1,…,N-1
};

// ─── Família de fase (φₙ / ψₙ) ──────────────────────────────────────────────
//
// As Eqs. (1a-d) incluem ângulos de fase arbitrários φₙ e ψₙ.  A simetria
// adicional do guia retangular (§2.1) acopla φₙ e ψₙ de modo que apenas
// duas famílias ortogonais sobrevivem:
//   phi0  → sin(nθ) para Ez, cos(nθ) para Hz   (família φₙ = 0)
//   phi90 → cos(nθ) para Ez, sin(nθ) para Hz   (família φₙ = π/2)
// Cada família produz modos distintos (e.g. HE₁₁ aparece em phi0, EH₁₁ em phi90).
// Veja docs/02.1_efeitos_da_simetria.md e docs/02.4_designacao_dos_modos.md.
enum class PhaseFamily
{
    phi0,   // φₙ = 0:    Ez ∝ sin(nθ),  Hz ∝ cos(nθ)
    phi90,  // φₙ = π/2:  Ez ∝ cos(nθ),  Hz ∝ sin(nθ)
};

// ─── Modo de cálculo do ponto de fronteira ────────────────────────────────────
//
// Ao avaliar as condições de contorno na fronteira retangular, é preciso
// converter o ângulo θ de um ponto de casamento em coordenada radial r.
//   intersection → r é a interseção geométrica do raio θ com o retângulo
//                   (r·cos θ = a/2 ou r·sin θ = b/2); física correta.
//   literal      → r é calculado por escalonamento trigonométrico direto;
//                   modo experimental/debug, produz pontos fora da borda.
enum class BoundaryGeometryMode
{
    literal,       // r = (a/2)·cos θ  ou  (b/2)·sin θ  — modo experimental
    intersection,  // r = (a/2)/cos θ  ou  (b/2)/sin θ  — interseção real com a borda
};

// ─── Modo de distribuição de pontos para paridade even / retângulo ────────────
//
// Para paridade even num guia quadrado, os conjuntos de pontos de casamento
// de Ez e Hz são naturalmente intercalados (§2.2).  Para um retângulo geral,
// o artigo original usa uma distribuição levemente diferente ("paper"):
//   paper        → segue exatamente §2.2 do artigo Goell
//   square_split → usa a mesma lógica do guia quadrado, mesmo para a/b ≠ 1
enum class EvenRectMatchingMode
{
    paper,        // distribuição de pontos conforme §2.2 do artigo
    square_split, // distribui pontos como no quadrado (alternativa)
};

// ─── Tipo de equação de contorno numa linha da matriz Q ──────────────────────
//
// Cada linha de Q impõe a continuidade de um componente de campo na fronteira
// do núcleo dielétrico.  Os quatro tipos cobrem as quatro equações de contorno
// necessárias (§2.3):
//   ez_long → continuidade de Ez  [Eqs. 7a, 7b]
//   hz_long → continuidade de Hz  [Eqs. 7c, 7d]
//   et_tan  → continuidade de Et  [Eqs. 7e-h]
//   ht_tan  → continuidade de Ht  [Eqs. 7i-l]
enum class RowKind
{
    ez_long, // continuidade da componente longitudinal Ez
    hz_long, // continuidade da componente longitudinal Hz
    et_tan,  // continuidade da componente tangencial Et
    ht_tan,  // continuidade da componente tangencial Ht
};

// ─── Parâmetros de execução do solver ────────────────────────────────────────
//
// Agrupa todas as opções configuráveis pelo CLI.  Os valores default reproduzem
// a configuração típica usada na validação da Tabela I do artigo.
// Veja docs/simbolos.md §"Parâmetros do CLI" para a tabela completa.
struct Params
{
    // Geometria do guia (§2, Fig. 2)
    double a_over_b = 1.0;   // razão de aspecto a/b (largura / altura do núcleo)
    double n_r      = 1.01;  // índice de refração relativo √(ε₁/ε₀) > 1

    // Resolução da expansão e varredura
    int N     = 5;    // número de harmônicos retidos na expansão [Eq. 1a-d]
    int Pscan = 160;  // pontos de varredura em P' por valor de B (0 < P' < 1)

    // Intervalo de frequência normalizada B = 2b/λ₀·√(n²_r−1)  [Eq. 6 / §2.6]
    double B_min = 0.1;
    double B_max = 4.0;
    int    NB    = 40;  // número de passos em B (NB=0 → apenas B_min)

    // Seleção de família de modos
    HarmonicParity parity = HarmonicParity::odd;       // paridade dos harmônicos
    PhaseFamily    phase  = PhaseFamily::phi0;         // família de fase φ₀ ou φ₉₀

    // Opções numéricas
    BoundaryGeometryMode geometry_mode  = BoundaryGeometryMode::intersection;
    EvenRectMatchingMode even_rect_mode = EvenRectMatchingMode::paper;
    std::string          metric         = "det";   // "det" ou "sv" (valor singular)
    std::string          det_search     = "sign";  // "sign" (mudança de sinal) ou "minima"
    bool all_minima       = true;   // exportar todos os modos por valor de B
    bool allow_edge_minima = false; // incluir mínimos nas bordas de P'
    bool rescale_matrix   = true;   // normalizar linhas e colunas de Q antes do cálculo

    // Modos de saída alternativos (diagnóstico)
    bool   dump_scan     = false; // varredura bruta merit(P') para B fixo
    double dump_B        = 0.0;   // valor de B usado no dump_scan
    bool   dump_det_sign = false; // incluir coluna det_sign no dump_scan
    bool   test_bessel   = false; // tabela de validação das funções de Bessel
    bool   null_vector   = false; // incluir Ez_frac, Hz_frac, mode_class no CSV [F3.1]

    // ── Modo de mapa de campo [F3.2] ──────────────────────────────────────
    // Ativo com --field-map: amostras Ez, Hz, Ex, Ey, Hx, Hy numa grade (x,y)
    // para um par (B, P') específico de um modo já localizado.
    // Saída: CSV com 12 colunas por ponto de grade.  Ver docs/simbolos.md §"CSV campo".
    bool   field_map    = false; // ativa modo de mapa de campo transversal
    double field_B      = 1.0;  // frequência normalizada B do modo  [Eq. 6]
    double field_Pprime = 0.5;  // constante de propagação P' do modo [Eq. 6]
    int    field_nx     = 60;   // pontos de grade em x
    int    field_ny     = 60;   // pontos de grade em y
    double field_margin = 1.5;  // extensão espacial = margin × meia-largura do núcleo
};

// Conversores de enum → string (usados nas colunas do CSV de saída).
std::string parity_name(HarmonicParity parity);
std::string phase_name(PhaseFamily phase);
std::string geometry_name(BoundaryGeometryMode mode);

#endif // GOELL_COMMON_HPP
