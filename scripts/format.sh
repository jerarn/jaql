#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${script_dir}/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/format.sh [--check] [file ...]

Formats tracked .hpp and .cpp files in-place by default.
Pass specific files to format only those paths.
Use --check for a non-mutating validation run.
EOF
}

mode="write"
file_args=()

while (($# > 0)); do
    case "$1" in
        --check)
            mode="check"
            shift
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        --)
            shift
            file_args+=("$@")
            break
            ;;
        -*)
            jaql_die "unknown argument '${1}'. See ./scripts/format.sh --help."
            ;;
        *)
            file_args+=("$1")
            shift
            ;;
    esac
done

repo_root="$(jaql_repo_root)"
clang_format_bin="$(jaql_find_first clang-format-17 clang-format || true)"
[[ -n "${clang_format_bin}" ]] || jaql_die "clang-format is required. Install clang-format-17 or clang-format."

if ((${#file_args[@]} > 0)); then
    files=("${file_args[@]}")
else
    mapfile -t files < <(git -C "${repo_root}" ls-files -- '*.hpp' '*.cpp')
fi
if ((${#files[@]} == 0)); then
    printf 'No tracked C++ headers or sources found.\n'
    exit 0
fi

printf 'Using %s for %d file(s)\n' "${clang_format_bin}" "${#files[@]}"
(
    cd -- "${repo_root}"
    if [[ "${mode}" == "check" ]]; then
        "${clang_format_bin}" --dry-run --Werror "${files[@]}"
    else
        "${clang_format_bin}" -i "${files[@]}"
    fi
)
