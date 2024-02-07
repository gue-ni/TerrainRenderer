cmake_minimum_required(VERSION 3.21)
project(TerrainRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
    glm GIT_REPOSITORY https://github.com/g-truc/glm.git
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 3b15fa82ea74739b574d705fea44959b58142eb8
)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(
    imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG master
)
FetchContent_MakeAvailable(imgui)

file(GLOB EXTERNAL_SOURCE
    ${imgui_SOURCE_DIR}/*.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

include_directories(${imgui_SOURCE_DIR})

add_subdirectory(collision)

if(WIN32)
    FetchContent_Declare(
        GLEW
        URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip
    )
    FetchContent_MakeAvailable(GLEW)

    FetchContent_Declare(
        SDL2
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.26.4/SDL2-devel-2.26.4-VC.zip
    )
    FetchContent_MakeAvailable(SDL2)

    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/build/_deps/sdl2-src")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/build/_deps/glew-src")
endif(WIN32)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)

add_executable(app
    src/main.cpp
    src/Window.cpp
    src/Game.cpp
    src/Clock.cpp

    gfx/gl.cpp
    gfx/image.cpp
    gfx/transform.cpp
    gfx/camera.cpp

    src/TileService.cpp
    src/TileCache.cpp
    src/TerrainRenderer.cpp
    src/QuadTree.cpp
    src/Cube.cpp
    src/Chunk.cpp

    ${EXTERNAL_SOURCE}
)

target_link_libraries(app PRIVATE collision ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} cpr::cpr)
target_include_directories(app PRIVATE "${CMAKE_SOURCE_DIR}/build/_deps/glm-src")

if(WIN32)
    add_custom_command(TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:app> $<TARGET_FILE_DIR:app>
        COMMAND_EXPAND_LISTS
        COMMENT "Copy *.dll"
    )

    add_custom_command(TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/build/_deps/glew-src/bin/Release/x64/glew32.dll" $<TARGET_FILE_DIR:app>
        COMMENT "Copy glew32.dll"
    )
endif(WIN32)

