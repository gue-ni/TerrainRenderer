cmake_minimum_required(VERSION 3.21)

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
)

target_include_directories(terrain PUBLIC
  "."
  "${glm_SOURCE_DIR}"
  "${SDL2_INCLUDE_DIRS}"
)
