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

add_subdirectory(gfx)
add_subdirectory(collision)

add_executable(app
    src/main.cpp
    src/Window.cpp
    src/Game.cpp
    src/Clock.cpp

    src/TileService.cpp
    src/TileCache.cpp
    src/TerrainRenderer.cpp
    src/QuadTree.cpp
    src/Cube.cpp
    src/Chunk.cpp

    ${EXTERNAL_SOURCE}
)

#target_link_libraries(app PRIVATE collision ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} cpr::cpr)
target_link_libraries(app PRIVATE collision gfx cpr::cpr)
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

