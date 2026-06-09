#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${script_dir}/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/check_docs.sh [--preset <preset>]

Runs Doxygen against public headers and fails on documentation warnings.
Requires a docs-enabled configure: ./scripts/bootstrap.sh --docs [--preset <preset>]
Defaults: preset=gcc-debug.
EOF
}

preset="$(jaql_default_preset)"

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
        *)
            jaql_die "unknown argument '${1}'. See ./scripts/check_docs.sh --help."
            ;;
    esac
done

repo_root="$(jaql_repo_root)"
mapfile -t headers < <(git -C "${repo_root}" ls-files -- 'include/jaql' | grep -E '\.hpp$' || true)
if ((${#headers[@]} == 0)); then
    printf 'No public headers found under include/jaql.\n'
    exit 0
fi

build_dir="$(jaql_require_build_dir "${preset}")"
cache_file="${build_dir}/CMakeCache.txt"
[[ -f "${cache_file}" ]] || jaql_die "missing ${cache_file}. Run ./scripts/bootstrap.sh --preset ${preset} --docs first."

if ! grep -q '^JAQL_BUILD_DOCS:BOOL=ON' "${cache_file}"; then
    jaql_die "JAQL_BUILD_DOCS is not enabled. Run ./scripts/bootstrap.sh --preset ${preset} --docs first."
fi

doxygen_executable="$(grep '^DOXYGEN_EXECUTABLE:' "${cache_file}" | cut -d= -f2-)"
[[ -n "${doxygen_executable}" && -x "${doxygen_executable}" ]] || jaql_die "Doxygen executable is missing or not executable (${doxygen_executable:-unset}). Re-run ./scripts/bootstrap.sh --preset ${preset} --docs."

printf 'Checking %d public header(s) with Doxygen (preset %s)\n' "${#headers[@]}" "${preset}"
(
    cd -- "${repo_root}"
    cmake --build --preset "${preset}" --target doxygen
)
