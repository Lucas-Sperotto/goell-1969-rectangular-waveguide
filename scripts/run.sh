#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT"

BIN="build/goell_q_solver"
CPP_SRC="src/goell_q_solver.cpp"
PYTHON_BIN="${PYTHON_BIN:-python3}"
MPLCONFIGDIR_DEFAULT="$REPO_ROOT/out/.mplconfig"
export MPLCONFIGDIR="${MPLCONFIGDIR:-$MPLCONFIGDIR_DEFAULT}"

mkdir -p build out figures "$MPLCONFIGDIR"

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
    echo "Eigen nao encontrado. Ajuste o include path em run.sh." >&2
    exit 1
  }

  g++ -O3 -std=c++17 "$CPP_SRC" -I "$eigen_dir" -o "$BIN"
}

solver_needs_rebuild() {
  [ ! -x "$BIN" ] || [ "$CPP_SRC" -nt "$BIN" ]
}

ensure_build() {
  if solver_needs_rebuild; then
    build_solver
  fi
}

figure_defaults() {
  case "$1" in
    fig16)
      printf '%s\n' "1" "1.0001" "Fig. 16 - razao de aspecto unitaria, Delta n_r -> 0"
      ;;
    fig17)
      printf '%s\n' "2" "1.0001" "Fig. 17 - a/b = 2, Delta n_r -> 0"
      ;;
    fig18)
      printf '%s\n' "1" "1.5" "Fig. 18 - razao de aspecto unitaria, Delta n_r = 0.5"
      ;;
    fig19)
      printf '%s\n' "2" "1.5" "Fig. 19 - a/b = 2, Delta n_r = 0.5"
      ;;
    *)
      echo "Figura desconhecida: $1" >&2
      exit 1
      ;;
  esac
}

run_raw_case() {
  local parity="$1"
  local phase="$2"
  local outfile="$3"

  local raw_n="${RAW_N:-5}"
  local raw_nb="${RAW_NB:-40}"
  local raw_pscan="${RAW_PSCAN:-240}"
  local metric="${RAW_METRIC:-det}"
  local det_search="${RAW_DET_SEARCH:-sign}"
  local geometry="${RAW_GEOMETRY:-intersection}"
  local even_rect_mode="${EVEN_RECT_MODE:-paper}"
  local rescale="${RAW_RESCALE:-off}"

  local args=(
    --parity "$parity"
    --phase "$phase"
    --geometry "$geometry"
    --a_over_b "$FIG_A_OVER_B"
    --nr "$FIG_NR"
    --N "$raw_n"
    --Bmin "$FIG_BMIN"
    --Bmax "$FIG_BMAX"
    --NB "$raw_nb"
    --Pscan "$raw_pscan"
    --metric "$metric"
    --det-search "$det_search"
    --even-rect-mode "$even_rect_mode"
    --all-minima
  )

  if [ "$rescale" = "off" ]; then
    args+=(--no-rescale)
  elif [ "$rescale" = "on" ]; then
    args+=(--rescale)
  fi

  "$BIN" "${args[@]}" > "$outfile"
}

run_table1() {
  ensure_build
  "$PYTHON_BIN" scripts/reproduce_table1.py \
    --mode "${TABLE1_MODE:-autonomous}" \
    --pscan "${TABLE1_PSCAN:-240}" \
    --even-rect-mode "${EVEN_RECT_MODE:-paper}" \
    "$@"
}

run_figure() {
  local tag="$1"
  shift || true

  if [ "${1:-}" = "--rebuild" ]; then
    build_solver
    shift
  else
    ensure_build
  fi

  mapfile -t defaults < <(figure_defaults "$tag")
  local FIG_A_OVER_B="${FIG_A_OVER_B:-${defaults[0]}}"
  local FIG_NR="${FIG_NR:-${defaults[1]}}"
  local FIG_TITLE="${FIG_TITLE:-${defaults[2]}}"
  local FIG_BMIN="${FIG_BMIN:-0}"
  local FIG_BMAX="${FIG_BMAX:-4}"

  local odd_phi0_csv="out/${tag}_odd_phi0.csv"
  local odd_phi90_csv="out/${tag}_odd_phi90.csv"
  local even_phi0_csv="out/${tag}_even_phi0.csv"
  local even_phi90_csv="out/${tag}_even_phi90.csv"
  local debug_png="figures/${tag}_debug_classes.png"
  local final_png="figures/${tag}_final.png"

  run_raw_case odd phi0 "$odd_phi0_csv"
  run_raw_case odd phi90 "$odd_phi90_csv"
  run_raw_case even phi0 "$even_phi0_csv"
  run_raw_case even phi90 "$even_phi90_csv"

  "$PYTHON_BIN" scripts/plot_compare.py \
    "$odd_phi0_csv" \
    "$odd_phi90_csv" \
    "$even_phi0_csv" \
    "$even_phi90_csv" \
    --scatter \
    --pmin 0.05 \
    --title "Goell CHM - ${FIG_TITLE} - debug por classe" \
    --save "$debug_png"

  "$PYTHON_BIN" scripts/validate_goell.py \
    --stability-figures "$tag" \
    --report-figures "$tag" \
    --n-values "${STABLE_N_VALUES:-5,7,9}" \
    --min-support "${STABLE_MIN_SUPPORT:-2}" \
    --p-tol "${STABLE_P_TOL:-0.03}" \
    --stability-pscan "${STABLE_PSCAN:-240}" \
    --det-search "${STABLE_DET_SEARCH:-sign}" \
    --even-rect-mode "${EVEN_RECT_MODE:-paper}" \
    --tracked-max-jump "${TRACKED_MAX_JUMP:-0.12}" \
    --tracked-min-length "${TRACKED_MIN_LENGTH:-6}" \
    --tracked-monotonic-tol "${TRACKED_MONOTONIC_TOL:-0.02}" \
    --tracked-min-monotonic-fraction "${TRACKED_MIN_MONOTONIC_FRACTION:-0.85}"

  "$PYTHON_BIN" scripts/plot_compare.py \
    "out/${tag}_stable_odd_phi0_tracked.csv" \
    "out/${tag}_stable_odd_phi90_tracked.csv" \
    "out/${tag}_stable_even_phi0_tracked.csv" \
    "out/${tag}_stable_even_phi90_tracked.csv" \
    --pmin 0.05 \
    --title "Goell CHM - ${FIG_TITLE} - curvas finais" \
    --save "$final_png"

  echo "Figura: $tag"
  echo "  a/b = $FIG_A_OVER_B"
  echo "  n_r = $FIG_NR"
  echo "Arquivos principais:"
  echo "  $odd_phi0_csv"
  echo "  $odd_phi90_csv"
  echo "  $even_phi0_csv"
  echo "  $even_phi90_csv"
  echo "  out/${tag}_stable_odd_phi0.csv"
  echo "  out/${tag}_stable_odd_phi90.csv"
  echo "  out/${tag}_stable_even_phi0.csv"
  echo "  out/${tag}_stable_even_phi90.csv"
  echo "  $debug_png"
  echo "  $final_png"
  echo "  out/${tag}_validation_summary.md"
}

run_principal_sweep() {
  local tag="$1"
  shift || true

  ensure_build

  local vary=""
  local values=""
  local sweep_a_over_b="${SWEEP_A_OVER_B:-1}"
  local sweep_nr="${SWEEP_NR:-1.0001}"
  local title=""

  case "$tag" in
    fig20)
      vary="nr"
      values="${FIG20_NR_VALUES:-1.0001,1.01,1.1,1.5,2,5,11}"
      sweep_a_over_b="${SWEEP_A_OVER_B:-1}"
      title="Goell CHM - Fig. 20 - modos principais vs n_r"
      ;;
    fig21)
      vary="nr"
      values="${FIG21_NR_VALUES:-1.0001,1.01,1.1,1.5,2,5,11}"
      sweep_a_over_b="${SWEEP_A_OVER_B:-2}"
      title="Goell CHM - Fig. 21 - modos principais vs n_r"
      ;;
    fig22)
      vary="a_over_b"
      values="${FIG22_ASPECT_VALUES:-1,1.5,2,3,4,6,8}"
      sweep_nr="${SWEEP_NR:-1.0001}"
      title="Goell CHM - Fig. 22 - modos principais vs a/b"
      ;;
    *)
      echo "Sweep principal desconhecido: $tag" >&2
      exit 1
      ;;
  esac

  local reuse_args=()
  if [ "${SWEEP_REUSE_EXISTING:-0}" = "1" ]; then
    reuse_args+=(--reuse-existing)
  fi

  "$PYTHON_BIN" scripts/sweep_principal_modes.py \
    --tag "$tag" \
    --vary "$vary" \
    --values "$values" \
    --a-over-b "$sweep_a_over_b" \
    --nr "$sweep_nr" \
    --n-values "${STABLE_N_VALUES:-5,7,9}" \
    --min-support "${STABLE_MIN_SUPPORT:-2}" \
    --p-tol "${STABLE_P_TOL:-0.03}" \
    --pscan "${STABLE_PSCAN:-240}" \
    --det-search "${STABLE_DET_SEARCH:-sign}" \
    --even-rect-mode "${EVEN_RECT_MODE:-paper}" \
    --tracked-max-jump "${TRACKED_MAX_JUMP:-0.12}" \
    --tracked-min-length "${TRACKED_MIN_LENGTH:-6}" \
    --tracked-monotonic-tol "${TRACKED_MONOTONIC_TOL:-0.02}" \
    --tracked-min-monotonic-fraction "${TRACKED_MIN_MONOTONIC_FRACTION:-0.85}" \
    --Bmin "${FIG_BMIN:-0}" \
    --Bmax "${FIG_BMAX:-4}" \
    --NB "${RAW_NB:-40}" \
    --title "$title" \
    "${reuse_args[@]}"
}

run_validate() {
  ensure_build

  "$PYTHON_BIN" scripts/reproduce_table1.py \
    --mode autonomous \
    --pscan "${TABLE1_PSCAN:-240}" \
    --even-rect-mode "${EVEN_RECT_MODE:-paper}" \
    --fail-on-threshold \
    --mae-threshold "${TABLE1_MAE_THRESHOLD:-0.01}" \
    --max-error-threshold "${TABLE1_MAX_ERROR_THRESHOLD:-0.02}"

  "$PYTHON_BIN" scripts/analyze_table1_variation.py

  "$PYTHON_BIN" scripts/validate_goell.py \
    --stability-figures fig16 fig17 \
    --report-figures fig16 fig17 \
    --n-values "${STABLE_N_VALUES:-5,7,9}" \
    --min-support "${STABLE_MIN_SUPPORT:-2}" \
    --p-tol "${STABLE_P_TOL:-0.03}" \
    --stability-pscan "${STABLE_PSCAN:-240}" \
    --det-search "${STABLE_DET_SEARCH:-sign}" \
    --even-rect-mode "${EVEN_RECT_MODE:-paper}" \
    --tracked-max-jump "${TRACKED_MAX_JUMP:-0.12}" \
    --tracked-min-length "${TRACKED_MIN_LENGTH:-6}" \
    --tracked-monotonic-tol "${TRACKED_MONOTONIC_TOL:-0.02}" \
    --tracked-min-monotonic-fraction "${TRACKED_MIN_MONOTONIC_FRACTION:-0.85}"
}

run_all_core() {
  run_table1
  run_figure fig16
  run_figure fig17
  run_figure fig18
  run_figure fig19
  run_validate
}

run_all_curves() {
  run_figure fig16
  run_figure fig17
  run_figure fig18
  run_figure fig19
  run_principal_sweep fig20
  run_principal_sweep fig21
  run_principal_sweep fig22
}

usage() {
  cat <<'EOF'
Uso: ./run.sh <comando>

Comandos:
  build
  table1
  fig16
  fig17
  fig18
  fig19
  fig20
  fig21
  fig22
  validate
  all-core
  all-curves
EOF
}

CMD="${1:-}"
if [ -z "$CMD" ]; then
  usage
  exit 1
fi
shift || true

case "$CMD" in
  build)
    build_solver
    ;;
  table1)
    run_table1 "$@"
    ;;
  fig16|fig17|fig18|fig19)
    run_figure "$CMD" "$@"
    ;;
  fig20|fig21|fig22)
    run_principal_sweep "$CMD" "$@"
    ;;
  validate)
    run_validate
    ;;
  all-core)
    run_all_core
    ;;
  all-curves)
    run_all_curves
    ;;
  *)
    usage
    exit 1
    ;;
esac
