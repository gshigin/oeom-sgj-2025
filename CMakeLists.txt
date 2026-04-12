cmake_minimum_required(VERSION 3.24)

# Super-repo: game sources in `game/`, engine `meh` as git submodule.
# `meh` must load first so all targets exist before `game/` (see meh/README.md).
project(
    AllEyesOnMe
    VERSION 0.0.0
    LANGUAGES CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# `meh` is a submodule; if configure fails, run: git submodule update --init --recursive
add_subdirectory(meh)
add_subdirectory(game)
