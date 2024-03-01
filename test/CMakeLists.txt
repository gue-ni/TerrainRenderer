cmake_minimum_required(VERSION 3.18)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests
  test_collision.cpp
  test_quadtree.cpp
  test_terrain.cpp
)

if(CMAKE_COMPILER_IS_GNUCC)
    target_link_options(tests PRIVATE -fsanitize=thread)
    target_compile_options(tests PRIVATE -fsanitize=thread)
endif()

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain collision terrain)

if(ENABLE_ADDRESS_SANITIZER)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

if(ENABLE_THREAD_SANITIZER)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=thread")
endif()

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
