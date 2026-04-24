/*
 * main.cpp — Ponto de entrada do solver Goell CHM.
 *
 * Fluxo de execução:
 *
 *   1. parse_args     — lê argv e preenche Params P
 *   2. Despacha para um dos três modos de saída:
 *
 *      a) --test-bessel → write_bessel_csv
 *         Tabela de validação de Jₙ, Kₙ e derivadas.
 *         Consumida por validate_bessel.py (70/70 pontos de referência).
 *
 *      b) --dump-scan B → write_dump_scan_csv
 *         Varredura bruta de merit(P') para B fixo.
 *         Útil para diagnosticar o perfil de det(Q) antes de ajustar Pscan.
 *
 *      c) (padrão) → write_root_csv
 *         Saída principal: curvas de propagação P'(B) para cada modo
 *         encontrado no intervalo [B_min, B_max].  Uma linha CSV por modo.
 *         Com --null-vector, inclui Ez_frac, Hz_frac, mode_class [F3.1].
 *
 * Saída em stdout (CSV); erros em stderr.  Código de saída: 0=OK, 1=erro.
 *
 * Exemplo de uso:
 *   ./build/goell_q_solver --parity odd --phase phi0 --a_over_b 1 --nr 1.5 \
 *     --N 5 --Bmin 1 --Bmax 4 --NB 30 --Pscan 200 > out/modos.csv
 *
 * Referência: docs/README.md  docs/simbolos.md  docs/02.7_metodo_de_computacao.md
 */

#include <exception>
#include <iomanip>
#include <iostream>

#include "goell/cli.hpp"
#include "goell/common.hpp"
#include "goell/output.hpp"
#include "goell/field.hpp"

int main(int argc, char **argv)
{
    // Desabilita sincronização com stdio C para maior velocidade de stdout.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Params P;
    try
    {
        parse_args(argc, argv, P);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[goell_q_solver] " << e.what() << "\n";
        return 1;
    }

    // Precisão alta para --test-bessel (comparação com tabelas de referência).
    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(15);

    if (P.test_bessel)
    {
        write_bessel_csv(std::cout);
        return 0;
    }

    // Precisão de 6 casas para a saída principal (suficiente para P' e B).
    std::cout.precision(6);

    if (P.dump_scan)
    {
        write_dump_scan_csv(std::cout, P);
        return 0;
    }

    // Modo de mapa de campo [F3.2]: avalia Ez, Hz, Et, Ht numa grade 2D para
    // um modo específico (field_B, field_Pprime).  Todos os cálculos em C++.
    if (P.field_map)
    {
        write_field_map_csv(std::cout, P);
        return 0;
    }

    // Modo principal: curvas de propagação P'(B)  [§3.3]
    write_root_csv(std::cout, P);
    return 0;
}
