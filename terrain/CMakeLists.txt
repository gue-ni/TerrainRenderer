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
