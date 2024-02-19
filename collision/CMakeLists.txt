cmake_minimum_required(VERSION 3.18)

FetchContent_Declare(
    glm 
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 33b0eb9fa336ffd8551024b1d2690e418014553b # v1.0.0
)
FetchContent_MakeAvailable(glm)

add_library(collision STATIC Collision.cpp Collision.h)

target_include_directories(collision PUBLIC "${glm_SOURCE_DIR}" ".")
