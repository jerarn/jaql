#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${script_dir}/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/lint.sh [--preset <preset>] [--cache] [--changed | --since <ref>] [-- <clang-tidy-args...>]

Runs clang-tidy against tracked .cpp translation units using the preset build directory.

Options:
  --preset <preset>  CMake preset whose compile database is used (default: gcc-debug).
  --cache            Route clang-tidy through clang-tidy-cache (ctcache) to reuse
                     results for unchanged translation units across runs.
  --changed          Lint only in-scope .cpp changed in the working tree
                     (staged + unstaged + untracked) versus HEAD.
  --since <ref>      Lint only in-scope .cpp changed versus the merge-base with <ref>
                     (e.g. --since origin/main).
  -- <args...>       Extra arguments forwarded to clang-tidy.

--changed and --since are mutually exclusive. With no diff flag, every in-scope unit is
linted. Benchmarks are always excluded (they are absent from the default compile
database). A diff that touches any header falls back to a full run, since a header can
affect many translation units.
EOF
}

preset="$(jaql_default_preset)"
extra_args=()
use_cache=false
diff_mode=""  # "" (full) | "changed" | "since"
since_ref=""

while (($# > 0)); do
    case "$1" in
        --preset)
            (($# >= 2)) || jaql_die "--preset requires a value."
            preset="$2"
            shift 2
            ;;
        --cache)
            use_cache=true
            shift
            ;;
        --changed)
            [[ -z "${diff_mode}" ]] || jaql_die "--changed and --since are mutually exclusive."
            diff_mode="changed"
            shift
            ;;
        --since)
            (($# >= 2)) || jaql_die "--since requires a <ref> value."
            [[ -z "${diff_mode}" ]] || jaql_die "--changed and --since are mutually exclusive."
            diff_mode="since"
            since_ref="$2"
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

# Full in-scope set: tracked .cpp excluding benchmarks/. Benchmarks are only compiled
# under the `benchmark` preset, so they are absent from the default preset's
# compile_commands.json; linting a file that is not in the compile database makes
# clang-tidy fall back to default flags, which fails to find google-benchmark headers
# and emits spurious diagnostics on macro-generated benchmark code.
mapfile -t all_files < <(git -C "${repo_root}" ls-files -- '*.cpp' ':(exclude)benchmarks/**')
if ((${#all_files[@]} == 0)); then
    printf 'No tracked C++ translation units found.\n'
    exit 0
fi

# Resolve the set of translation units to lint based on the diff mode.
files=()
if [[ -z "${diff_mode}" ]]; then
    files=("${all_files[@]}")
else
    changed=()
    if [[ "${diff_mode}" == "changed" ]]; then
        mapfile -t changed < <(
            {
                git -C "${repo_root}" diff --name-only --diff-filter=d HEAD
                git -C "${repo_root}" ls-files --others --exclude-standard
            } | sort -u
        )
    else
        git -C "${repo_root}" rev-parse --verify --quiet "${since_ref}^{commit}" >/dev/null \
            || jaql_die "--since ref '${since_ref}' not found locally. Fetch it first (e.g. git fetch origin)."
        mapfile -t changed < <(
            git -C "${repo_root}" diff --name-only --diff-filter=d "${since_ref}...HEAD" | sort -u
        )
    fi

    # A changed header can affect many translation units, so linting only the changed
    # .cpp would miss regressions. Fall back to the full in-scope set instead.
    header_changed=false
    for path in "${changed[@]}"; do
        case "${path}" in
            *.hpp | *.h | *.hh | *.hxx | *.ipp)
                header_changed=true
                break
                ;;
        esac
    done

    if [[ "${header_changed}" == true ]]; then
        printf 'Diff touches a header; linting the full in-scope set.\n'
        files=("${all_files[@]}")
    else
        # Collect changed .cpp, excluding benchmarks/ by path. Tracked/in-database
        # membership is enforced by the compile-database intersection below, which also
        # warns about brand-new files that have not been configured yet.
        for path in "${changed[@]}"; do
            case "${path}" in
                benchmarks/*) ;;
                *.cpp) files+=("${path}") ;;
            esac
        done
    fi

    if ((${#files[@]} == 0)); then
        printf 'No changed in-scope C++ translation units; nothing to lint.\n'
        exit 0
    fi
fi

build_dir="$(jaql_require_compile_commands "${preset}")"

# In diff mode the selected set may contain files absent from the compile database
# (e.g. a brand-new .cpp not yet configured into CMake). Linting those would trigger
# the same default-flags fallback that breaks on benchmarks, so drop them with a warning.
if [[ -n "${diff_mode}" ]]; then
    mapfile -t db_files < <(jaql_compile_db_files "${build_dir}")
    declare -A in_db=()
    for path in "${db_files[@]}"; do
        in_db["${path}"]=1
    done
    kept=()
    for path in "${files[@]}"; do
        if [[ -n "${in_db["${path}"]:-}" ]]; then
            kept+=("${path}")
        else
            printf 'warning: %s is not in the compile database; skipping. Reconfigure with: cmake --preset %s\n' \
                "${path}" "${preset}" >&2
        fi
    done
    files=("${kept[@]}")
    if ((${#files[@]} == 0)); then
        printf 'No changed translation units present in the compile database; nothing to lint.\n'
        exit 0
    fi
fi

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

# Optionally front the clang-tidy binary with clang-tidy-cache (ctcache), which keys on
# the preprocessed source plus arguments and reuses results across runs.
tidy_cmd=( "${clang_tidy_bin}" )
if [[ "${use_cache}" == true ]]; then
    cache_bin="$(jaql_find_first clang-tidy-cache || true)"
    [[ -n "${cache_bin}" ]] \
        || jaql_die "--cache requires clang-tidy-cache (ctcache). Install with: pip install clang-tidy-cache"
    tidy_cmd=( "${cache_bin}" "${clang_tidy_bin}" )
    export CTCACHE_CLANG_TIDY="${clang_tidy_bin}"
    # ctcache does not reliably hash .clang-tidy itself, so salt the cache directory with
    # a fingerprint of the active config files and the clang-tidy version. A config or
    # toolchain change then naturally misses stale entries.
    fingerprint="$(
        {
            "${clang_tidy_bin}" --version 2>/dev/null
            cat "${repo_root}/.clang-tidy" 2>/dev/null
            cat "${repo_root}/tests/.clang-tidy" 2>/dev/null
        } | sha256sum | cut -d' ' -f1
    )"
    export CTCACHE_DIR="${CTCACHE_DIR:-${HOME}/.cache/ctcache}/jaql-${fingerprint:0:16}"
    printf 'Caching clang-tidy results via %s (CTCACHE_DIR=%s)\n' "${cache_bin}" "${CTCACHE_DIR}"
fi

jobs="$(nproc 2>/dev/null || echo 1)"
printf 'Running %s across %d translation unit(s) (-j %s) with compile database %s\n' \
    "${clang_tidy_bin}" "${#files[@]}" "${jobs}" "${build_dir}/compile_commands.json"

# clang-tidy processes its file arguments sequentially, so a single invocation over
# every translation unit pins the run to one core and dominates the wall time. Fan the
# units out across cores instead: each is an independent invocation sharing the same
# compile database and arguments. xargs exits non-zero if any unit fails, which
# `set -o pipefail` + `set -e` turn into a script failure.
(
    cd -- "${repo_root}"
    printf '%s\0' "${files[@]}" | xargs -0 -P "${jobs}" -n 1 \
        "${tidy_cmd[@]}" -p "${build_dir}" \
        --header-filter="^${repo_root}/(include|src)/.*" \
        --quiet \
        "${gcc_install_args[@]}" \
        "${extra_args[@]}"
)
