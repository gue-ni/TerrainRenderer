cmake_minimum_required(VERSION 3.18)

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG e69e5f977d458f2650bb346dadf2ad30c5320281
)
FetchContent_MakeAvailable(fmt)

find_package(Threads REQUIRED)

add_library(terrain STATIC
    TileService.cpp TileService.h
    TileCache.cpp TileCache.h
    TerrainRenderer.cpp TerrainRenderer.h
    QuadTree.cpp QuadTree.h
    Cube.cpp Cube.h
    Chunk.cpp Chunk.h
    TileUtils.h
)

target_link_libraries(terrain PUBLIC
  collision
  gfx
  cpr::cpr
  fmt::fmt
  Threads::Threads
)

target_include_directories(terrain PUBLIC
  "."
  "${glm_SOURCE_DIR}"
  "${SDL2_INCLUDE_DIRS}"
)

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/generated/sky.vert" "${CMAKE_CURRENT_SOURCE_DIR}/generated/sky.frag" "${CMAKE_CURRENT_SOURCE_DIR}/generated/terrain.vert" "${CMAKE_CURRENT_SOURCE_DIR}/generated/terrain.frag"
  COMMAND ${CMAKE_COMMAND} -DSHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}" -P ${CMAKE_CURRENT_SOURCE_DIR}/parse_shader.cmake
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sky.vert" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sky.frag" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/terrain.vert" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/terrain.frag"
  COMMENT "Generate shaders"
)

add_custom_target(shaders 
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/sky.vert" "${CMAKE_CURRENT_SOURCE_DIR}/generated/sky.frag" "${CMAKE_CURRENT_SOURCE_DIR}/generated/terrain.vert" "${CMAKE_CURRENT_SOURCE_DIR}/generated/terrain.frag"
)

add_dependencies(terrain shaders)
