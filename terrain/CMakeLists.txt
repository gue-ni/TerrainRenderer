cmake_minimum_required(VERSION 3.21)

add_library(terrain STATIC
    TileService.cpp
    TileCache.cpp
    TerrainRenderer.cpp
    QuadTree.cpp
    Cube.cpp
    Chunk.cpp
)

target_link_libraries(terrain PUBLIC
  collision
  gfx
  cpr::cpr
)

target_include_directories(terrain PUBLIC
  "."
  "${glm_SOURCE_DIR}"
  "${SDL2_INCLUDE_DIRS}"
)
