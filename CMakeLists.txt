cmake_minimum_required(VERSION 3.20)
project(OpenGL_Particle C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_PREFIX_PATH "/home/utopia010/libs/glfw/install")
add_subdirectory(
    /home/utopia010/libs/glad/
    ${CMAKE_BINARY_DIR}/glad
    )
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)

add_executable(a.out src/main.cpp)
target_link_libraries(a.out PRIVATE glfw OpenGL::GL glad::glad)

add_executable(test src/test.cpp)
target_link_libraries(test PRIVATE glfw OpenGL::GL glad::glad)