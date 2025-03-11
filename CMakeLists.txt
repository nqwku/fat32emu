cmake_minimum_required(VERSION 3.30)
project(fat32emu C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
endif()

set(SOURCES
        src/main.c
        src/fat32.c
        src/disk.c
        src/commands.c
        src/utils.c
)

add_executable(f32disk ${SOURCES})

install(TARGETS f32disk DESTINATION bin)

option(BUILD_TESTING "Build the testing tree" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

