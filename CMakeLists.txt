# ---- Project info
cmake_minimum_required(VERSION 3.14)

project(FluidSimulation C CXX)

# ---- Package imports
find_package(glfw3 REQUIRED)

# ---- Compile glad into its own library
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

# Add ImGui files
set(IMGUI_SOURCES
    Imgui/imgui.cpp
    Imgui/imgui_draw.cpp
    Imgui/imgui_widgets.cpp
    Imgui/imgui_tables.cpp
    Imgui/imgui_impl_glfw.cpp
    Imgui/imgui_impl_opengl3.cpp
)

# ---- Compile your executable
add_executable(out
    src/utils.cpp
    src/interaction.cpp
    src/fluides.cpp
    src/display.cpp
    src/matrix.cpp
    src/main.cpp
    src/fluidRender.cpp
    src/fluid_solver.cpp
    ${IMGUI_SOURCES}
)

# Include headers from src/
target_include_directories(out PRIVATE 
    src
    Imgui
)

# Link libraries
target_link_libraries(out glad glfw)