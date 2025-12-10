# ---- Project info

# Determines minimum cmake version required, mandatory (but pretty useless) line
cmake_minimum_required(VERSION 3.14)

# Defines the name of this project, and assert that it will use C++
project(FluidSimulation C CXX)



# ---- Package imports

# glfw3 should be installed on your system prior to this line for it to work
find_package(glfw3 REQUIRED)



# ---- Compile glad into its own library

# Generate a static glad library. Now you can use the name "glad" further in the code, it will refer to that new library
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)



# ---- Compile your executable

# Create an executable named "out" that will depend on the source file src/main.cpp
add_executable(out src/main.cpp)

# Link "glad" and "glfw" to your "out" executable
target_link_libraries(out glad glfw)