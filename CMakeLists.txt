# ---- Project info
cmake_minimum_required(VERSION 3.14)

project(FluidSimulation C CXX)

# ---- Package imports
find_package(glfw3 REQUIRED)
find_package(PkgConfig QUIET)

# Try to find SDL2 with pkg-config if PkgConfig is available
if(PkgConfig_FOUND)
    pkg_check_modules(SDL2 sdl2)
    pkg_check_modules(SDL2_MIXER SDL2_mixer)
endif()

#à ajouter quand ils seront incorporés au projet et qu'on pourra les demander
#find_package(SDL2 REQUIRED)
#find_package(SDL2_mixer REQUIRED)

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
    serial/serialib.cpp
)

# Include headers from src/
target_include_directories(out PRIVATE 
    src
    Imgui
)

# Link libraries
target_link_libraries(out glad glfw)


# Link SDL2 only if found
if(SDL2_FOUND)
    target_link_libraries(out ${SDL2_LIBRARIES})
    target_include_directories(out PRIVATE ${SDL2_INCLUDE_DIRS})
endif()
if(SDL2_MIXER_FOUND)
    target_link_libraries(out ${SDL2_MIXER_LIBRARIES})
    target_include_directories(out PRIVATE ${SDL2_MIXER_INCLUDE_DIRS})
endif()

