cmake_minimum_required(VERSION 3.27)
project(SibGamJam)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

add_subdirectory(external)

add_executable(AllEyesOnMe src/main.cpp resources.rc)
target_link_libraries(AllEyesOnMe PRIVATE SDL3_image::SDL3_image SDL3::SDL3 SDL3_ttf::SDL3_ttf)

add_custom_target(clear_assets ALL
    COMMAND ${CMAKE_COMMAND} -E rm -r
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
    COMMENT "Clearing assets..."
)

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
    COMMENT "Copying assets..."
)

add_dependencies(AllEyesOnMe copy_assets)
add_dependencies(copy_assets clear_assets)

if(WIN32)
    set_target_properties(AllEyesOnMe PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()