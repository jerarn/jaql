# Local Conan Profiles

JAQL ships CI-parity profiles under `profiles/ci/` for reproducible dependency resolution.

## Default behaviour

`./scripts/bootstrap.sh` selects a host profile from the CMake preset:

| Preset family | Host profile |
|---------------|--------------|
| `gcc-*`, `ci-gcc-*`, `asan`, `ubsan`, `tsan`, `benchmark` | `profiles/ci/gcc13` |
| `clang-*`, `ci-clang-*` | `profiles/ci/clang17-libcxx` |

`build_type` is always passed at install time (`-s build_type=Debug|Release`) and is not baked into the profile.

## Overrides

Use `--host-profile` / `--build-profile`, or set `JAQL_CONAN_HOST_PROFILE` / `JAQL_CONAN_BUILD_PROFILE`.

To use your machine-detected compiler instead of the CI profiles, pass `--host-profile default`. Bootstrap will run `conan profile detect` if no `default` profile exists.
