cmake_minimum_required(VERSION 3.20)
project(genvista CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Threads REQUIRED)

add_executable(genvista
    src/main.cpp
    src/noise.cpp
    src/solids.cpp
    src/shade.cpp
    src/render.cpp
    src/ink.cpp
    src/palette.cpp
    src/vista.cpp
    src/land.cpp
    src/trees.cpp
    src/clouds.cpp
    src/picture.cpp
    src/flyover.cpp
    src/stb_write.cpp
)
target_compile_options(genvista PRIVATE -Wall -Wextra -O2 -march=native)
target_link_libraries(genvista PRIVATE Threads::Threads)
