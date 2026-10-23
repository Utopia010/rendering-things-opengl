from conan import ConanFile
from conan.tools.cmake import cmake_layout

class ProjectConfig(ConanFile):
    generators = "CMakeToolchain", "CMakeDeps"
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        # 你的两个直接依赖
        depend_list = [
            "glm/1.0.3",
            "glfw/3.4",
            "glad/2.0.8",
            "assimp/6.0.5",
        ]
        [self.requires(lib) for lib in depend_list]
        self.requires("stb/cci.20240531", override=True)

    def layout(self):
        cmake_layout(self, build_folder="build")
