#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${script_dir}/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/lint.sh [--preset <preset>] [-- <clang-tidy-args...>]

Runs clang-tidy against tracked .cpp translation units using the preset build directory.
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

printf 'Running %s with compile database %s\n' "${clang_tidy_bin}" "${build_dir}/compile_commands.json"
(
    cd -- "${repo_root}"
    "${clang_tidy_bin}" -p "${build_dir}" "${extra_args[@]}" "${files[@]}"
)
