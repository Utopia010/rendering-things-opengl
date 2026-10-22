cmake_minimum_required(VERSION 3.20)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
project(OpenGL_Particle C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# glfw
find_package(glfw3 REQUIRED)

# glad
find_package(glad REQUIRED)

# glm
find_package(glm REQUIRED)

# imgui
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC)
set(imgui_backend_source_files "imgui_impl_opengl3.cpp" "imgui_impl_glfw.cpp")
file(GLOB imgui_source_files "${imgui_SOURCE_DIR}/*.cpp")
target_sources(imgui PRIVATE ${imgui_source_files})

foreach(file IN ITEMS ${imgui_backend_source_files})
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/${file})
endforeach()

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw)

# my class
add_subdirectory(./MyClass)

find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)

# src
add_subdirectory(src)

add_executable(a.out main.cpp)
target_link_libraries(a.out PRIVATE
    OpenGL::GL glad::glad
    glm::glm myClass assimp::assimp
    imgui)
