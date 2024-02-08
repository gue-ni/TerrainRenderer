cmake_minimum_required(VERSION 3.21)

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

file(GLOB imgui_SOURCE
    ${imgui_SOURCE_DIR}/*.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

include_directories(${imgui_SOURCE_DIR})

add_executable(app
    main.cpp
    Window.cpp
    Game.cpp
    Clock.cpp
    TileService.cpp
    TileCache.cpp
    TerrainRenderer.cpp
    QuadTree.cpp
    Cube.cpp
    Chunk.cpp
    ${imgui_SOURCE}
)

target_link_libraries(app PRIVATE collision gfx cpr::cpr)

target_include_directories(app PRIVATE
    "${glm_SOURCE_DIR}"
    "${SDL2_INCLUDE_DIRS}"
)

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


