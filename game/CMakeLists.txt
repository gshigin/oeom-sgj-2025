# Game executable — super-repo pattern: link engine first, then SDL on the binary
# (see README.md and meh/README.md).

add_executable(
    AllEyesOnMe
    src/main.cpp
)

target_include_directories(AllEyesOnMe PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(
    AllEyesOnMe
    PRIVATE
        meh::compile_options
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        SDL3::SDL3
)

# Ship assets next to the binary so runtime paths like "assets/..." work from the exe dir
# (Debug/Release on multi-config generators included via TARGET_FILE_DIR).
add_custom_command(
    TARGET AllEyesOnMe
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:AllEyesOnMe>/assets"
    COMMENT "Copy game assets next to AllEyesOnMe"
)

if(WIN32)
    target_sources(AllEyesOnMe PRIVATE resources.rc)
    set_target_properties(
        AllEyesOnMe
        PROPERTIES
            WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()
