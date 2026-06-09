import os

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain


class JaqlConan(ConanFile):
    name = "jaql"
    version = "0.1.0"
    description = "Modular C++23 quantitative finance library"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "build_docs": [True, False],
        "build_tests": [True, False],
        "build_benchmarks": [True, False],
    }
    default_options = {
        "build_docs": False,
        "build_tests": True,
        "build_benchmarks": False,
    }
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("tl-expected/1.2.0")
        self.requires("spdlog/1.17.0")
        self.requires("eigen/5.0.1")
        self.requires("nlohmann_json/3.12.0")

    def build_requirements(self):
        if self.options.build_docs:
            self.tool_requires("doxygen/1.17.0")
        if self.options.build_tests:
            self.test_requires("gtest/1.17.0")
        if self.options.build_benchmarks:
            self.test_requires("benchmark/1.9.5")

    def generate(self):
        toolchain = CMakeToolchain(self)
        # Keep repository presets as the single source of truth.
        toolchain.user_presets_path = False
        if self.options.build_docs:
            # Propagate the Conan-installed Doxygen path and the docs toggle
            # into the CMake cache so no extra -D flags are needed at configure.
            doxygen_pkg = self.dependencies.build["doxygen"]
            toolchain.variables["DOXYGEN_EXECUTABLE"] = os.path.join(
                doxygen_pkg.package_folder, "bin", "doxygen"
            )
            toolchain.variables["JAQL_BUILD_DOCS"] = True
        toolchain.generate()
