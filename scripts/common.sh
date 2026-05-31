#!/usr/bin/env bash

set -euo pipefail

# Raises an error.
jaql_die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

# Returns root directory path.
jaql_repo_root() {
    local script_dir
    script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
    cd -- "${script_dir}/.." && pwd
}

# Returns default preset name.
jaql_default_preset() {
    printf '%s\n' "${JAQL_PRESET:-jaql-debug}"
}

# Returns build directory path.
jaql_build_dir() {
    local preset="$1"
    printf '%s/build/%s\n' "$(jaql_repo_root)" "${preset}"
}

# Returns CMake build type.
jaql_preset_build_type() {
    local preset="$1"

    case "${preset}" in
        jaql-debug|asan|ubsan|tsan|ci-linux)
            printf 'Debug\n'
            ;;
        jaql-release|benchmark)
            printf 'Release\n'
            ;;
        *)
            jaql_die "unsupported preset '${preset}'. Use one of: jaql-debug, jaql-release, ci-linux, asan, ubsan, tsan, benchmark."
            ;;
    esac
}

# Checks that the command is available.
jaql_require_command() {
    local command_name="$1"
    local help_text="$2"

    if ! command -v "${command_name}" >/dev/null 2>&1; then
        jaql_die "${command_name} is required. ${help_text}"
    fi
}

# Finds the first available command.
jaql_find_first() {
    local candidate

    for candidate in "$@"; do
        if command -v "${candidate}" >/dev/null 2>&1; then
            printf '%s\n' "${candidate}"
            return 0
        fi
    done

    return 1
}

# Checks that the build directory exists.
jaql_require_build_dir() {
    local preset="$1"
    local build_dir

    build_dir="$(jaql_build_dir "${preset}")"
    if [[ ! -d "${build_dir}" ]]; then
        jaql_die "build directory '${build_dir}' does not exist. Run ./scripts/bootstrap.sh --preset ${preset} first."
    fi

    printf '%s\n' "${build_dir}"
}

# Checks that compile_commands.json exists.
jaql_require_compile_commands() {
    local preset="$1"
    local build_dir

    build_dir="$(jaql_require_build_dir "${preset}")"
    if [[ ! -f "${build_dir}/compile_commands.json" ]]; then
        jaql_die "missing ${build_dir}/compile_commands.json. Re-run ./scripts/bootstrap.sh --preset ${preset} or cmake --preset ${preset}."
    fi

    printf '%s\n' "${build_dir}"
}

# Ensures that the specified Conan profile exists.
jaql_ensure_conan_profile() {
    local profile_name="$1"

    if conan profile path "${profile_name}" >/dev/null 2>&1; then
        return 0
    fi

    if [[ "${profile_name}" == "default" ]]; then
        printf 'Conan profile '\''default'\'' not found. Detecting it now.\n'
        conan profile detect --force >/dev/null
        return 0
    fi

    jaql_die "Conan profile '${profile_name}' was not found. Create it with Conan or pass a different profile."
}
