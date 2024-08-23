cmake_minimum_required(VERSION 3.5)
project(chip8)

# expose includes to lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# compile with debug information
# set(CMAKE_CXX_FLAGS "g")

# use system sdl, not vendored one
find_package(SDL2 REQUIRED)
include_directories(SDL2Test ${SDL2_INCLUDE_DIRS})

add_executable(chip8
  src/main.cpp
)

target_link_libraries(chip8 ${SDL2_LIBRARIES})
