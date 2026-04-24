# Documentacao

Esta pasta concentra a traducao revisada do artigo de Goell, as notas permanentes de apoio e os ativos usados para consulta.

## Navegacao Rapida

- [README geral](../README.md)
- [PDF original do artigo](j.1538-7305.1969.tb01168.x.pdf)
- [Pendencias tecnicas do projeto](../TODO.md)

## Trilha Principal

1. [00_titulo_resumo.md](00_titulo_resumo.md)
2. [01_introducao.md](01_introducao.md)
3. [02_derivacao_das_equacoes.md](02_derivacao_das_equacoes.md)
4. [02.1_efeitos_da_simetria.md](02.1_efeitos_da_simetria.md)
5. [02.2_selecao_dos_pontos_de_ajuste.md](02.2_selecao_dos_pontos_de_ajuste.md)
6. [02.3_formulacao_dos_elementos_de_matriz.md](02.3_formulacao_dos_elementos_de_matriz.md)
7. [02.4_designacao_dos_modos.md](02.4_designacao_dos_modos.md)
8. [02.5_diferencas_entre_as_funcoes_de_campo_eletrico_e_magnetico.md](02.5_diferencas_entre_as_funcoes_de_campo_eletrico_e_magnetico.md)
9. [02.6_normalizacao.md](02.6_normalizacao.md)
10. [02.7_metodo_de_computacao.md](02.7_metodo_de_computacao.md)
11. [03_resultados_do_calculo.md](03_resultados_do_calculo.md)
12. [03.1_precisao.md](03.1_precisao.md)
13. [03.2_configuracoes_modais.md](03.2_configuracoes_modais.md)
14. [03.3_curvas_de_propagacao.md](03.3_curvas_de_propagacao.md)
15. [04_conclusoes.md](04_conclusoes.md)
16. [05_agradecimentos.md](05_agradecimentos.md)
17. [06_referencias.md](06_referencias.md)

## Lista de Símbolos

- [simbolos.md](simbolos.md): todos os símbolos do projeto — geometria, eletromagnetismo, variáveis normalizadas, funções especiais, blocos da matriz Q, vetor nulo e parâmetros do CLI — com correspondência artigo ↔ docs ↔ código C++.

## Materiais De Apoio

- [referencias/01_expansoes_de_campo.md](referencias/01_expansoes_de_campo.md): folha rapida para expansoes, simetria e pontos de casamento.
- [referencias/02_matriz_global_e_normalizacao.md](referencias/02_matriz_global_e_normalizacao.md): resumo dos blocos da matriz `Q`, determinante global e variaveis normalizadas.
- [referencias/03_checklist_de_conferencia.md](referencias/03_checklist_de_conferencia.md): roteiro de revisao do PDF e dos pontos mais sensiveis da notacao.
- [referencias/04_notas_sobre_a_tabela_1.md](referencias/04_notas_sobre_a_tabela_1.md): nota do repositorio sobre a reproducao numerica da Tabela I.
- [referencias/05_plano_de_codificacao.md](referencias/05_plano_de_codificacao.md): plano de codificacao e criterios de aceite do pipeline atual.
- A infraestrutura de campos agora vive no solver modular (`include/goell/field.hpp`, `src/core/field.cpp`) e o utilitario de plotagem correspondente e `scripts/field_map.py`.

## Convencao Da Pasta

- Os arquivos `00` a `06` formam a traducao principal do artigo.
- Os arquivos em `referencias/` sao notas auxiliares permanentes do repositorio.
- A pasta [img](img/) guarda as figuras citadas na traducao.
- O solver oficial correspondente a estes docs agora vive em `include/goell/`, `src/core/` e `src/main.cpp`, com wrappers publicos em `scripts/`.
- O backlog tecnico foi consolidado em [../TODO.md](../TODO.md), para nao misturar pendencias de implementacao com a trilha de leitura.
