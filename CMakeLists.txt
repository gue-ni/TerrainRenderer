cmake_minimum_required(VERSION 3.21)
project(TerrainRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

add_subdirectory(gfx)
add_subdirectory(collision)
add_subdirectory(src)
add_subdirectory(test)

