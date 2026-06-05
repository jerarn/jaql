from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain


class JaqlConan(ConanFile):
    name = "jaql"
    version = "0.1.0"
    description = "Modular C++23 quantitative finance library"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("tl-expected/1.1.0")
        self.requires("spdlog/1.13.0")
        self.requires("eigen/3.4.0")
        self.requires("nlohmann_json/3.11.3")

    def build_requirements(self):
        self.test_requires("gtest/1.14.0")
        self.test_requires("benchmark/1.8.3")

    def generate(self):
        toolchain = CMakeToolchain(self)
        # Keep repository presets as the single source of truth.
        toolchain.user_presets_path = False
        toolchain.generate()
