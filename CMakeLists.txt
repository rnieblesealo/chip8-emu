cmake_minimum_required(VERSION 3.5)
project(chip8emu)

# expose includes to lsp
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# use system sdl, not vendored one

find_package(SDL2 REQUIRED)
include_directories(SDL2Test ${SDL2_INCLUDE_DIRS})

add_executable(chip8emu
  src/core.cpp
  src/platform.cpp
)

target_link_libraries(chip8emu ${SDL2_LIBRARIES})
