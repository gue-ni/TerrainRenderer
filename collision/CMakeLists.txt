cmake_minimum_required(VERSION 3.21)

FetchContent_Declare(
    glm GIT_REPOSITORY https://github.com/g-truc/glm.git
)
FetchContent_MakeAvailable(glm)

add_library(collision STATIC Collision.cpp Collision.h)

target_include_directories(collision PUBLIC "${glm_SOURCE_DIR}" ".")
