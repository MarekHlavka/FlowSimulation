cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1 )

project(PGPa)

set(GLEW_LIBRARIES libs/glew-2.2.0)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)


add_subdirectory(libs/SDL2-2.28.4)
add_subdirectory(libs/geGL-master)
add_subdirectory(libs/glm-0.9.9.8)
add_subdirectory(libs/glfw-3.3.8)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})

# ImGui -----------------------------------------------------
set(IMGUI_PATH ${CMAKE_SOURCE_DIR}/libs/imgui-docking/)
file(GLOB IMGUI_GLOB
    ${IMGUI_PATH}/imgui.h
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/imconfig.h
    ${IMGUI_PATH}/imgui_demo.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_internal.h
    ${IMGUI_PATH}/imstb_rectpack.h
    ${IMGUI_PATH}/imstb_textedit.h
    ${IMGUI_PATH}/imstb_truetype.h
    ${IMGUI_PATH}/imgui_tables.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp

    # specific bindings...
    ${IMGUI_PATH}/backends/imgui_impl_glfw.h
    ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdl2.h
    ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.h
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3_loader.cpp)
add_library("ImGui" STATIC ${IMGUI_GLOB})
target_link_libraries("ImGui" PRIVATE glfw SDL2::SDL2-static)
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH})
# ImGui -----------------------------------------------------

# Create your game executable target as usual
add_executable(${PROJECT_NAME} flow.cpp
    shaderLoader.hpp shaderLoader.cpp
    particles.hpp particles.cpp
    grid.hpp grid.cpp)

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2-static glfw glm ImGui GLEW::GLEW ${OPENGL_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/libs)
target_include_directories(${PROJECT_NAME} PUBLIC ImGui)

add_custom_target(run ./${PROJECT_NAME})