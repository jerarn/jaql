#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${script_dir}/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/lint.sh [--preset <preset>] [-- <clang-tidy-args...>]

Runs clang-tidy against tracked .cpp translation units using the preset build directory.
Defaults: preset=gcc-debug.
EOF
}

preset="$(jaql_default_preset)"
extra_args=()

while (($# > 0)); do
    case "$1" in
        --preset)
            (($# >= 2)) || jaql_die "--preset requires a value."
            preset="$2"
            shift 2
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        --)
            shift
            extra_args=("$@")
            break
            ;;
        *)
            jaql_die "unknown argument '${1}'. See ./scripts/lint.sh --help."
            ;;
    esac
done

repo_root="$(jaql_repo_root)"
mapfile -t files < <(git -C "${repo_root}" ls-files -- '*.cpp')
if ((${#files[@]} == 0)); then
    printf 'No tracked C++ translation units found.\n'
    exit 0
fi

build_dir="$(jaql_require_compile_commands "${preset}")"
clang_tidy_bin="$(jaql_find_first clang-tidy-17 clang-tidy || true)"
[[ -n "${clang_tidy_bin}" ]] || jaql_die "clang-tidy is required. Install clang-tidy-17 or clang-tidy."

# Pin clang-tidy to the same GCC install as the build compiler so it uses the
# correct libstdc++ headers rather than auto-detecting the highest GCC version
# on the system (which may differ between local and CI, e.g. ubuntu-24.04
# pre-installs GCC14 alongside an explicitly requested GCC13).
gcc_install_args=()
_cxx="${CXX:-$(command -v g++ 2>/dev/null || true)}"
if [[ -n "${_cxx}" ]]; then
    _gcc_dir="$("${_cxx}" --print-search-dirs 2>/dev/null | awk '/^install:/{print $2}')"
    _gcc_dir="${_gcc_dir%/}"  # strip trailing slash for a clean path
    if [[ -n "${_gcc_dir}" ]] && [[ -d "${_gcc_dir}" ]]; then
        gcc_install_args=( "--extra-arg=--gcc-install-dir=${_gcc_dir}" )
        printf 'Anchoring clang-tidy GCC headers to: %s\n' "${_gcc_dir}"
    fi
fi

printf 'Running %s with compile database %s\n' "${clang_tidy_bin}" "${build_dir}/compile_commands.json"
(
    cd -- "${repo_root}"
    "${clang_tidy_bin}" -p "${build_dir}" \
        --header-filter="^${repo_root}/(include|src)/.*" \
        "${gcc_install_args[@]}" \
        "${extra_args[@]}" "${files[@]}"
)
