# ---- Project info
cmake_minimum_required(VERSION 3.14)

project(FluidSimulation C CXX)

# ---- Package imports
find_package(glfw3 REQUIRED)

# ---- Compile glad into its own library
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

# ---- Compile your executable
add_executable(out
    src/utils.cpp
    src/interaction.cpp
    src/fluides.cpp
    src/display.cpp
    src/main.cpp
)

# Include headers from src/
target_include_directories(out PRIVATE src)

# Link libraries
target_link_libraries(out glad glfw)