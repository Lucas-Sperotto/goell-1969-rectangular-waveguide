#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

BIN="build/goell_q_solver"
OUTDIR="out"
FIGDIR="figures"
PYTHON_BIN="${PYTHON_BIN:-python3}"

FIGURE="${1:-fig16}"
REBUILD="${2:-}"

mkdir -p build "$OUTDIR" "$FIGDIR"

find_eigen_include() {
  for d in /usr/include/eigen3 /usr/local/include/eigen3 /opt/homebrew/include/eigen3 /mingw64/include/eigen3; do
    if [ -d "$d" ]; then
      printf '%s\n' "$d"
      return 0
    fi
  done
  return 1
}

build_solver() {
  local eigen_dir
  eigen_dir="$(find_eigen_include)" || {
    echo "Eigen nao encontrado. Ajuste o include path em src/presets.sh." >&2
    exit 1
  }
  g++ -O3 -std=c++17 src/goell_q_solver.cpp -I "$eigen_dir" -o "$BIN"
}

if [ ! -x "$BIN" ] || [ "$FIGURE" = "--rebuild" ] || [ "$REBUILD" = "--rebuild" ]; then
  build_solver
fi

case "$FIGURE" in
  --rebuild|"")
    FIGURE="fig16"
    ;;
esac

figure_defaults() {
  case "$1" in
    fig16)
      printf '%s\n' "1" "1.01" "Fig. 16 — razao de aspecto unitaria, diferenca de indice tendendo a zero"
      ;;
    fig17)
      printf '%s\n' "2" "1.01" "Fig. 17 — a/b = 2, diferenca de indice tendendo a zero"
      ;;
    fig18)
      printf '%s\n' "1" "1.5" "Fig. 18 — razao de aspecto unitaria, Delta n_r = 0.5"
      ;;
    fig19)
      printf '%s\n' "2" "1.5" "Fig. 19 — a/b = 2, Delta n_r = 0.5"
      ;;
    *)
      echo "Uso: bash src/presets.sh [fig16|fig17|fig18|fig19] [--rebuild]" >&2
      exit 1
      ;;
  esac
}

mapfile -t defaults < <(figure_defaults "$FIGURE")

# Sec. 3.2 / Fig. 4: o artigo diz que cinco harmônicos foram usados nos resultados.
A_OVER_B="${A_OVER_B:-${defaults[0]}}"
NR="${NR:-${defaults[1]}}"
TITLE="${TITLE:-${defaults[2]}}"
OUT_PREFIX="${OUT_PREFIX:-$FIGURE}"
BMIN="${BMIN:-0}"
BMAX="${BMAX:-4}"
N="${N:-5}"
NB="${NB:-40}"
PSCAN="${PSCAN:-160}"
# Para comparacao com o paper, o criterio por determinante fica mais fiel a eq. (19).
METRIC="${METRIC:-det}"
ALL_MINIMA="${ALL_MINIMA:-1}"
PLOT_MERIT_MAX="${PLOT_MERIT_MAX:-}"
RESCALE_MATRIX="${RESCALE_MATRIX:-auto}"
# A leitura "intersection" da fronteira reproduziu melhor a Tabela I do paper.
GEOMETRY_MODE="${GEOMETRY_MODE:-intersection}"

if [ -z "$PLOT_MERIT_MAX" ] && [ "$METRIC" = "sv" ]; then
  PLOT_MERIT_MAX="-8"
fi

case_value() {
  local prefix="$1"
  local suffix="$2"
  local default_value="$3"
  local name="${prefix}_${suffix}"
  printf '%s' "${!name:-$default_value}"
}

run_case() {
  local prefix="$1"
  local parity="$2"
  local phase="$3"
  local outfile="$4"

  local n nb pscan metric rescale_mode geometry_mode
  n="$(case_value "$prefix" N "$N")"
  nb="$(case_value "$prefix" NB "$NB")"
  pscan="$(case_value "$prefix" PSCAN "$PSCAN")"
  metric="$(case_value "$prefix" METRIC "$METRIC")"
  rescale_mode="$(case_value "$prefix" RESCALE_MATRIX "$RESCALE_MATRIX")"
  geometry_mode="$(case_value "$prefix" GEOMETRY_MODE "$GEOMETRY_MODE")"

  local args=(
    --parity "$parity"
    --phase "$phase"
    --geometry "$geometry_mode"
    --a_over_b "$A_OVER_B"
    --nr "$NR"
    --N "$n"
    --Bmin "$BMIN"
    --Bmax "$BMAX"
    --NB "$nb"
    --Pscan "$pscan"
    --metric "$metric"
  )

  if [ "$ALL_MINIMA" = "1" ]; then
    args+=(--all-minima)
  fi

  if [ "$rescale_mode" = "off" ]; then
    args+=(--no-rescale)
  elif [ "$rescale_mode" = "on" ]; then
    args+=(--rescale)
  elif [ "$rescale_mode" = "auto" ] && [ "$metric" = "det" ]; then
    # Para metric=det, o reescalonamento altera bastante a paisagem dos minimos
    # mesmo sem mover os zeros exatos. Para a busca por minimos, desligar o
    # reescalonamento ficou mais proximo do comportamento esperado do paper.
    args+=(--no-rescale)
  fi

  "$BIN" "${args[@]}" > "$outfile"
}

ODD_PHI0_CSV="$OUTDIR/${OUT_PREFIX}_odd_phi0.csv"
ODD_PHI90_CSV="$OUTDIR/${OUT_PREFIX}_odd_phi90.csv"
EVEN_PHI0_CSV="$OUTDIR/${OUT_PREFIX}_even_phi0.csv"
EVEN_PHI90_CSV="$OUTDIR/${OUT_PREFIX}_even_phi90.csv"
PLOT_FILE="$FIGDIR/${OUT_PREFIX}_classes.png"

run_case ODD_PHI0 odd phi0 "$ODD_PHI0_CSV"
run_case ODD_PHI90 odd phi90 "$ODD_PHI90_CSV"
run_case EVEN_PHI0 even phi0 "$EVEN_PHI0_CSV"
run_case EVEN_PHI90 even phi90 "$EVEN_PHI90_CSV"

plot_args=()
if [ "$ALL_MINIMA" = "1" ]; then
  plot_args+=(--scatter)
fi
if [ -n "$PLOT_MERIT_MAX" ]; then
  plot_args+=(--merit-max "$PLOT_MERIT_MAX")
fi

"$PYTHON_BIN" src/plot_compare.py \
  "$ODD_PHI0_CSV" \
  "$ODD_PHI90_CSV" \
  "$EVEN_PHI0_CSV" \
  "$EVEN_PHI90_CSV" \
  --title "Goell CHM - $TITLE" \
  "${plot_args[@]}" \
  --pmin 0.001 \
  --save "$PLOT_FILE"

echo "Figura: $FIGURE"
echo "  a/b = $A_OVER_B"
echo "  n_r = $NR"
echo "  N = $N, NB = $NB, Pscan = $PSCAN, metric = $METRIC"
echo "  RESCALE_MATRIX = $RESCALE_MATRIX"
echo "  GEOMETRY_MODE = $GEOMETRY_MODE"
echo "Arquivos gerados:"
echo "  $ODD_PHI0_CSV"
echo "  $ODD_PHI90_CSV"
echo "  $EVEN_PHI0_CSV"
echo "  $EVEN_PHI90_CSV"
echo "  $PLOT_FILE"
