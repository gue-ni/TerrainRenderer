cmake_minimum_required(VERSION 3.21)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests
  test-collision.cpp
  test-quadtree.cpp
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain collision terrain)

if(WIN32)
    add_custom_command(TARGET tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:tests> $<TARGET_FILE_DIR:tests>
        COMMAND_EXPAND_LISTS
        COMMENT "Copy *.dll"
    )

    add_custom_command(TARGET tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/_deps/glew-src/bin/Release/x64/glew32.dll" $<TARGET_FILE_DIR:tests>
        COMMENT "Copy glew32.dll"
    )
endif(WIN32)



