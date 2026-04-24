/*
 * field.hpp — Avaliação dos campos eletromagnéticos em grade 2D (F3.2).
 *
 * Dado o vetor nulo v = [aₙ | bₙ | cₙ | dₙ]ᵀ de Q no ponto modal (B, P'),
 * este módulo reconstrói os seis componentes de campo em qualquer ponto (x, y)
 * da seção transversal do guia.
 *
 * ─── Física (artigo §2, Eqs. 1-3) ─────────────────────────────────────────
 *
 * Os campos em coordenadas cilíndricas (r, θ) são derivados diretamente das
 * Eqs. (3a-d) e (1a-d):
 *
 *   Componentes longitudinais (reais em z=0, t=0):
 *     Ez_in  = Σ aₙ · Jₙ(h·r) · S(n,θ)   [Eq. 1a]
 *     Hz_in  = Σ bₙ · Jₙ(h·r) · C(n,θ)   [Eq. 1b]
 *     Ez_out = Σ cₙ · Kₙ(p·r) · S(n,θ)   [Eq. 1c]
 *     Hz_out = Σ dₙ · Kₙ(p·r) · C(n,θ)   [Eq. 1d]
 *
 *   Componentes transversais (avaliadas na fase ωt=π/2 em que são reais):
 *     Er, Eθ, Hr, Hθ — expressões detalhadas em field.cpp [Eqs. 3a-d]
 *
 *   Componentes cartesianas:
 *     Ex = Er·cos θ − Eθ·sin θ
 *     Ey = Er·sin θ + Eθ·cos θ   (análogo para H)
 *
 * ─── Normalização ──────────────────────────────────────────────────────────
 *
 * Todos os campos são normalizados pelo maior |v|² do núcleo para que os
 * mapas diferentes (Ez, Hz, Et, Ht) sejam comparáveis em escala relativa.
 * O campo Ez ou Hz dominante fica com pico ±1 no interior do guia.
 *
 * Referência: docs/simbolos.md §"Vetor nulo"  docs/02.5_diferencas_entre_as_funcoes_de_campo_eletrico_e_magnetico.md
 */

#ifndef GOELL_FIELD_HPP
#define GOELL_FIELD_HPP

#include "goell/common.hpp"
#include "goell/layout.hpp"

#include <Eigen/Dense>
#include <vector>

// ─── Ponto de campo na grade 2D ──────────────────────────────────────────────
//
// Armazena a posição (x, y) e todos os componentes do campo modal nesse ponto.
// Coordenadas em unidades de b (altura do núcleo): núcleo ocupa
//   x ∈ [−a/(2b), a/(2b)]  =  [−a_over_b/2, a_over_b/2]
//   y ∈ [−0.5, 0.5]
struct FieldPoint
{
    double x = 0.0, y = 0.0; // posição Cartesiana (unidades de b)
    bool   inside = false;    // true se (x,y) está dentro do núcleo retangular

    // Componentes longitudinais [Eqs. 1a-d]
    double Ez = 0.0; // componente z do campo elétrico
    double Hz = 0.0; // componente z do campo magnético

    // Componentes transversais cilíndricas [Eqs. 3a-d, fase ωt=π/2]
    double Er     = 0.0; // componente radial de E
    double Etheta = 0.0; // componente azimutal de E
    double Hr     = 0.0; // componente radial de H
    double Htheta = 0.0; // componente azimutal de H

    // Componentes transversais cartesianas (convertidas de Er, Eθ)
    double Ex = 0.0, Ey = 0.0; // campo elétrico transversal Cartesiano
    double Hx = 0.0, Hy = 0.0; // campo magnético transversal Cartesiano
};

// Avalia todos os componentes de campo no ponto (x, y) para o modo dado por v.
// L, h_scaled, p_scaled e kz_over_k0 devem ser pré-computados de (P, B, Pprime).
FieldPoint eval_field_point(
    const Params        &P,
    const ColumnLayout  &L,
    double               h_scaled,
    double               p_scaled,
    double               kz_over_k0,
    const Eigen::VectorXd &v,
    double               x,
    double               y);

// Constrói a grade completa de field_nx × field_ny pontos para o modo (B, Pprime).
// Obtém o vetor nulo v internamente via SVD.
// Retorna os pontos em ordem (x varia mais rápido, y mais devagar):
//   índice = iy * field_nx + ix.
std::vector<FieldPoint> compute_field_grid(
    const Params &P,
    double        B,
    double        Pprime);

#endif // GOELL_FIELD_HPP
