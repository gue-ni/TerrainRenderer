cmake_minimum_required(VERSION 3.18)

include(FetchContent)

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

set(SHADER_SOURCE
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sky.vert" 
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/sky.frag" 
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/terrain.vert" 
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/terrain.frag"
)
set(SHADER_GEN 
  "${CMAKE_CURRENT_SOURCE_DIR}/generated/sky.vert" 
  "${CMAKE_CURRENT_SOURCE_DIR}/generated/sky.frag" 
  "${CMAKE_CURRENT_SOURCE_DIR}/generated/terrain.vert" 
  "${CMAKE_CURRENT_SOURCE_DIR}/generated/terrain.frag"
)

add_custom_command(
  OUTPUT ${SHADER_GEN}
  COMMAND ${CMAKE_COMMAND} -DSHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}" -P ${CMAKE_CURRENT_SOURCE_DIR}/parse_shader.cmake
  DEPENDS ${SHADER_SOURCE} 
  COMMENT "Generate shaders"
)

add_custom_target(shaders DEPENDS ${SHADER_GEN})

add_dependencies(terrain shaders)
