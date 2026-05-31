#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${script_dir}/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/check_headers.sh [--preset <preset>]

Builds one standalone translation unit per public header using the preset Conan toolchain.
Defaults: preset=jaql-debug.
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
            jaql_die "unknown argument '${1}'. See ./scripts/check_headers.sh --help."
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
toolchain_file="${build_dir}/conan_toolchain.cmake"
[[ -f "${toolchain_file}" ]] || jaql_die "missing ${toolchain_file}. Run ./scripts/bootstrap.sh --preset ${preset} first."

work_dir="${build_dir}/header-check"
sources_dir="${work_dir}/sources"
build_type="$(jaql_preset_build_type "${preset}")"

rm -rf "${work_dir}"
mkdir -p "${sources_dir}"

index=0
for header in "${headers[@]}"; do
    source_file="${sources_dir}/header_${index}.cpp"
    printf '#include <%s>\n' "${header}" > "${source_file}"
    index=$((index + 1))
done

cat > "${work_dir}/CMakeLists.txt" <<'EOF'
cmake_minimum_required(VERSION 3.25)

project(jaql_header_check LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(tl-expected REQUIRED)
find_package(spdlog REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(jaql_header_deps INTERFACE)
target_include_directories(jaql_header_deps INTERFACE "${JAQL_SOURCE_DIR}/include")
target_link_libraries(
    jaql_header_deps
    INTERFACE
        tl::expected
        spdlog::spdlog
        Eigen3::Eigen
        nlohmann_json::nlohmann_json
)

file(GLOB header_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/sources/*.cpp")

foreach(source IN LISTS header_sources)
    get_filename_component(target_name "${source}" NAME_WE)
    add_library("${target_name}" OBJECT "${source}")
    target_link_libraries("${target_name}" PRIVATE jaql_header_deps)
endforeach()
EOF

printf 'Checking %d public header(s) with preset %s\n' "${#headers[@]}" "${preset}"
cmake -S "${work_dir}" -B "${work_dir}/build" \
    -DCMAKE_BUILD_TYPE="${build_type}" \
    -DJAQL_SOURCE_DIR="${repo_root}" \
    -DCMAKE_TOOLCHAIN_FILE="${toolchain_file}"
cmake --build "${work_dir}/build"
