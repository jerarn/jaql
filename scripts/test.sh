#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${script_dir}/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/test.sh [--preset <preset>] [-- <ctest-args...>]

Runs ctest with the selected preset and failure output enabled.
Defaults: preset=jaql-debug.
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
            jaql_die "unknown argument '${1}'. See ./scripts/test.sh --help."
            ;;
    esac
done

repo_root="$(jaql_repo_root)"
jaql_require_command ctest "Install CMake's test tools and ensure ctest is on PATH."

printf 'Running ctest preset %s\n' "${preset}"
(
    cd -- "${repo_root}"
    ctest --preset "${preset}" --output-on-failure "${extra_args[@]}"
)
