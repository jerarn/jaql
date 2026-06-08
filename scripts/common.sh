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
    printf '%s\n' "${JAQL_PRESET:-gcc-debug}"
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
        gcc-debug|clang-debug|asan|ubsan|tsan|ci-gcc-debug|ci-clang-debug)
            printf 'Debug\n'
            ;;
        gcc-release|clang-release|benchmark|ci-gcc-release|ci-clang-release)
            printf 'Release\n'
            ;;
        *)
            jaql_die "unsupported preset '${preset}'. Use one of: gcc-debug, gcc-release, clang-debug, clang-release, ci-gcc-debug, ci-clang-debug, ci-gcc-release, ci-clang-release, asan, ubsan, tsan, benchmark."
            ;;
    esac
}

# Returns the default Conan host profile for a CMake preset.
jaql_preset_host_profile() {
    local preset="$1"

    case "${preset}" in
        clang-debug|clang-release|ci-clang-debug|ci-clang-release)
            printf 'profiles/ci/clang17-libcxx\n'
            ;;
        gcc-debug|gcc-release|ci-gcc-debug|ci-gcc-release|asan|ubsan|tsan|benchmark)
            printf 'profiles/ci/gcc13\n'
            ;;
        *)
            jaql_die "unsupported preset '${preset}'. Use one of: gcc-debug, gcc-release, clang-debug, clang-release, ci-gcc-debug, ci-clang-debug, ci-gcc-release, ci-clang-release, asan, ubsan, tsan, benchmark."
            ;;
    esac
}

# Prints Conan option flags (-o jaql/*:...) for a CMake preset.
jaql_preset_conan_options() {
    local preset="$1"
    local build_tests build_benchmarks

    case "${preset}" in
        benchmark)
            build_tests=False
            build_benchmarks=True
            ;;
        *)
            build_tests=True
            build_benchmarks=False
            ;;
    esac

    printf '%s\n' "-o" "jaql/*:build_tests=${build_tests}" "-o" "jaql/*:build_benchmarks=${build_benchmarks}"
}

# Checks that the command is available.
jaql_require_command() {
    local command_name="$1"
    local help_text="$2"

    if ! command -v "${command_name}" >/dev/null 2>&1; then
        jaql_die "${command_name} is required. ${help_text}"
    fi
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

# Resolves a Conan profile to an absolute path when it lives in the repository.
jaql_resolve_conan_profile() {
    local profile_name="$1"
    local repo_root profile_path

    repo_root="$(jaql_repo_root)"
    profile_path="${repo_root}/${profile_name}"
    if [[ -f "${profile_path}" ]]; then
        printf '%s\n' "${profile_path}"
        return 0
    fi

    if [[ -f "${profile_name}" ]]; then
        printf '%s\n' "${profile_name}"
        return 0
    fi

    printf '%s\n' "${profile_name}"
}

# Ensures that the specified Conan profile exists.
jaql_ensure_conan_profile() {
    local profile_name="$1"
    local resolved_profile

    resolved_profile="$(jaql_resolve_conan_profile "${profile_name}")"
    if [[ -f "${resolved_profile}" ]]; then
        return 0
    fi

    if conan profile path "${profile_name}" >/dev/null 2>&1; then
        return 0
    fi

    if [[ "${profile_name}" == "default" ]]; then
        printf 'Conan profile '\''default'\'' not found. Detecting it now.\n'
        conan profile detect --force >/dev/null
        return 0
    fi

    jaql_die "Conan profile '${profile_name}' was not found. Use a repo profile under profiles/, create one with Conan, or pass --host-profile default."
}
