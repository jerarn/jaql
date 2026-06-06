#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${script_dir}/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/bootstrap.sh [--preset <preset>] [--host-profile <profile>] [--build-profile <profile>]

Installs Conan dependencies into the matching preset build directory and configures CMake.
Defaults: preset=gcc-debug, host-profile=default, build-profile=default.
EOF
}

preset="$(jaql_default_preset)"
host_profile="${JAQL_CONAN_HOST_PROFILE:-default}"
build_profile="${JAQL_CONAN_BUILD_PROFILE:-default}"

while (($# > 0)); do
    case "$1" in
        --preset)
            (($# >= 2)) || jaql_die "--preset requires a value."
            preset="$2"
            shift 2
            ;;
        --host-profile)
            (($# >= 2)) || jaql_die "--host-profile requires a value."
            host_profile="$2"
            shift 2
            ;;
        --build-profile)
            (($# >= 2)) || jaql_die "--build-profile requires a value."
            build_profile="$2"
            shift 2
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            jaql_die "unknown argument '${1}'. See ./scripts/bootstrap.sh --help."
            ;;
    esac
done

repo_root="$(jaql_repo_root)"
build_dir="$(jaql_build_dir "${preset}")"
build_type="$(jaql_preset_build_type "${preset}")"
user_presets_file="${repo_root}/CMakeUserPresets.json"

jaql_require_command cmake "Install CMake 3.25+ and ensure it is on PATH."
jaql_require_command ninja "Install Ninja 1.11+ and ensure it is on PATH."
jaql_require_command conan "Install Conan 2 and ensure it is on PATH."

jaql_ensure_conan_profile "${host_profile}"
jaql_ensure_conan_profile "${build_profile}"

mkdir -p "${build_dir}"

if [[ -f "${user_presets_file}" ]]; then
    printf 'Removing stale %s to avoid CMake preset collisions\n' "${user_presets_file}"
    rm -f -- "${user_presets_file}"
fi

printf 'Installing Conan dependencies into %s\n' "${build_dir}"
(
    cd -- "${repo_root}"
    conan install . \
        --output-folder "${build_dir}" \
        --build=missing \
        -pr:h "${host_profile}" \
        -pr:b "${build_profile}" \
        -s build_type="${build_type}"
)

printf 'Configuring CMake preset %s\n' "${preset}"
(
    cd -- "${repo_root}"
    cmake --preset "${preset}"
)
