# Contributing

Obrigado por considerar uma contribuicao para este repositorio.

O objetivo do projeto e reproduzir, com clareza, rastreabilidade e fidelidade, o artigo de J. E. Goell (1969) sobre analise por harmonicos circulares de guias de onda dieletricos retangulares. Por isso, contribuicoes tecnicamente corretas e cientificamente bem documentadas sao mais valiosas do que mudancas grandes ou rapidas.

## Antes de contribuir

- Leia o [README.md](README.md) para entender o escopo atual do repositorio.
- Leia [docs/README.md](docs/README.md) para navegar pela traducao e pelas notas permanentes.
- Consulte [TODO.md](TODO.md) para verificar o estado das pendencias tecnicas.
- Se a mudanca afetar regras operacionais para agentes automatizados, leia tambem [AGENTS.md](AGENTS.md).

## Principios do projeto

- Priorize clareza, didatica e fidelidade ao artigo original.
- Nao invente formulas ausentes. Quando houver lacuna, registre a incerteza explicitamente.
- Marque ambiguidades de OCR, traducao ou notacao com `TODO`.
- Preserve a rastreabilidade entre fisica, matematica, implementacao e documentacao.
- Preserve o nucleo numerico em `C++17`, sem dependencias desnecessarias.
- Garanta que o fluxo principal continue exportando resultados numericos em `out/` e figuras em `figures/`.
- Use os utilitarios Python em `scripts/` para plotagem, reproducao e diagnostico.
- Registre, sempre que possivel, quais equacoes, figuras ou tabelas do artigo cada modulo implementa.

## Estrutura esperada

- `README.md`: mapa geral do repositorio.
- `TODO.md`: backlog tecnico consolidado.
- `docs/`: traducao principal do artigo.
- `docs/referencias/`: notas auxiliares permanentes do repositorio.
- `run.sh`: interface publica para build, reproducao e validacao.
- `src/goell_q_solver.cpp`: solver principal.
- `scripts/*.py`: utilitarios de reproducao, validacao e plotagem.
- `src/presets.sh`: shim de compatibilidade para o fluxo antigo.
- `memory/`: refatoracao C++ modular experimental, ainda fora do build oficial.
- `build/`, `out/` e `figures/`: artefatos gerados localmente.

## Estilo de edicao

- Prefira comentarios curtos e tecnicos, especialmente em trechos numericos ou fisicos menos obvios.
- Em arquivos Markdown, use `$...$` para matematica inline e `$$...$$` para blocos.
- Se renomear ou mover arquivos em `docs/`, atualize tambem `docs/README.md` e os links do `README.md` da raiz.

## Build e verificacao

Os comandos principais do fluxo atual sao:

```bash
./run.sh build
./run.sh table1
./run.sh fig16
./run.sh fig20
./run.sh validate
python3 -m py_compile scripts/plot_compare.py scripts/reproduce_table1.py scripts/analyze_table1_variation.py scripts/validate_goell.py scripts/track_roots.py scripts/principal_modes.py scripts/sweep_principal_modes.py scripts/solver_api.py scripts/validate_bessel.py
```

Se a sua mudanca mexer na camada numerica ou nos scripts, rode ao menos o build e a verificacao mais proxima do trecho afetado.

## Mudancas cientificas e numericas

Ao alterar formulas, criterios de guiamento, interpretacoes de OCR ou parametros dos casos:

- explique no pull request quais equacoes ou figuras foram afetadas;
- diga se a mudanca altera resultados em `out/` ou `figures/`;
- explicite se a mudanca corrige bug, refina aproximacao ou muda interpretacao editorial;
- inclua `TODO` quando ainda houver dependencia de verificacao no artigo ou no scan.

## Artefatos gerados

Este repositorio usa `build/`, `out/` e `figures/` como artefatos locais de compilacao e reproducao. Em geral, eles nao devem ser commitados; quando um resultado precisar ser preservado, ele deve ser promovido conscientemente para documentacao ou nota permanente.

## Pull requests

Um bom pull request aqui costuma conter:

- objetivo claro e escopo limitado;
- referencia ao trecho do artigo, figura ou tabela afetada;
- resumo do impacto numerico ou documental;
- observacoes sobre riscos, limites ou pendencias restantes.

## Sobre `AGENTS.md`

`AGENTS.md` deve ser mantido enquanto o repositorio continuar usando agentes automatizados ou fluxos assistidos por IA. Ele complementa este guia com restricoes e prioridades especificas para automacao; nao substitui o `README.md` nem este `CONTRIBUTING.md`.
