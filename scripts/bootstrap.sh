#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${script_dir}/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/bootstrap.sh [--preset <preset>] [--host-profile <profile>] [--build-profile <profile>] [--docs]

Installs Conan dependencies into the matching preset build directory and configures CMake.
Defaults: preset=gcc-debug; host/build profiles are chosen from the preset unless overridden.
EOF
}

preset="$(jaql_default_preset)"
host_profile=""
build_profile=""
build_docs=false

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
        --docs)
            build_docs=true
            shift
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

if [[ -z "${host_profile}" ]]; then
    host_profile="${JAQL_CONAN_HOST_PROFILE:-$(jaql_preset_host_profile "${preset}")}"
fi
if [[ -z "${build_profile}" ]]; then
    build_profile="${JAQL_CONAN_BUILD_PROFILE:-${host_profile}}"
fi

repo_root="$(jaql_repo_root)"
build_dir="$(jaql_build_dir "${preset}")"
build_type="$(jaql_preset_build_type "${preset}")"
user_presets_file="${repo_root}/CMakeUserPresets.json"
lockfile="${repo_root}/conan.lock"
host_profile_resolved="$(jaql_resolve_conan_profile "${host_profile}")"
build_profile_resolved="$(jaql_resolve_conan_profile "${build_profile}")"

jaql_require_command cmake "Install CMake 3.25+ and ensure it is on PATH."
jaql_require_command ninja "Install Ninja 1.11+ and ensure it is on PATH."
jaql_require_command conan "Install Conan 2.4+ and ensure it is on PATH."

jaql_ensure_conan_profile "${host_profile}"
if [[ "${build_profile_resolved}" != "${host_profile_resolved}" ]]; then
    jaql_ensure_conan_profile "${build_profile}"
fi

mkdir -p "${build_dir}"

if [[ -f "${user_presets_file}" ]]; then
    printf 'Removing stale %s to avoid CMake preset collisions\n' "${user_presets_file}"
    rm -f -- "${user_presets_file}"
fi

printf 'Installing Conan dependencies into %s\n' "${build_dir}"
(
    cd -- "${repo_root}"
    conan_install_args=(
        install .
        --output-folder "${build_dir}"
        --build=missing
        -pr:h "${host_profile_resolved}"
        -pr:b "${build_profile_resolved}"
        -s build_type="${build_type}"
        -s:b build_type=Release
    )

    if [[ -f "${lockfile}" ]]; then
        conan_install_args+=( --lockfile "${lockfile}" --lockfile-partial )
    fi

    mapfile -t preset_conan_options < <(jaql_preset_conan_options "${preset}")
    conan_install_args+=( "${preset_conan_options[@]}" )

    if [[ "${build_docs}" == true ]]; then
        conan_install_args+=( -o "jaql/*:build_docs=True" )
    fi

    conan "${conan_install_args[@]}"
)

printf 'Configuring CMake preset %s\n' "${preset}"
(
    cd -- "${repo_root}"
    # Unset Conan-injected cache entries so a changed --docs flag or refreshed
    # toolchain path (e.g. after a new Doxygen binary is installed) is picked up.
    cmake --preset "${preset}" -UDOXYGEN_EXECUTABLE -UJAQL_BUILD_DOCS
)
