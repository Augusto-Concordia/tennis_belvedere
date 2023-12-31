cmake_minimum_required(VERSION 3.20)
project(tennis_belvedere)

set(CMAKE_CXX_STANDARD 20)

IF (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
ENDIF()

# finds our own OpenGL dependency from installed binaries
find_package(OpenGL REQUIRED)

# includes vendors' source CMake projects
add_subdirectory(vendor)

# add tennis_belvedere project and link glfw to it
file(GLOB_RECURSE TENNIS_BELVEDERE_FILES source/**.cpp)
add_executable(tennis_belvedere ${TENNIS_BELVEDERE_FILES})

target_include_directories(tennis_belvedere PRIVATE source)

IF (WIN32)
    target_link_libraries(tennis_belvedere PRIVATE glfw OpenGL::GL glm glad stb_image -static-libgcc -static-libstdc++)
ELSE()
    target_link_libraries(tennis_belvedere PRIVATE glfw OpenGL::GL glm glad stb_image)
ENDIF()