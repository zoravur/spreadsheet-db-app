cmake_minimum_required(VERSION 3.10)
project(hello_world)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

# include directories
include_directories(
        ./glad/include
        ./imgui
        ./imgui/backends
        /usr/include/freetype2
        /usr/include/libpng16
)

# find packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)
pkg_check_modules(FREETYPE2 REQUIRED freetype2)

find_package(fmt REQUIRED)


# add definitions
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

# source files
set(SRCS
        main2.cpp
        glad/src/glad.c
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_opengl3.cpp
        FrameRateCalculator.hpp
        InputController.cpp
        InputController.hpp
#        Model.cpp
#        Model.hpp
#        View.cpp
#        View.hpp
        Text.hpp
        Model.hpp
)


# set the path to the config file
set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/config.json")

# define the path to the destination directory
set(DEST_DIR "${CMAKE_BINARY_DIR}")

# copy the config file to the build directory
file(COPY ${CONFIG_FILE} DESTINATION ${DEST_DIR})

# add a custom command to copy the file
add_custom_command(
        OUTPUT "${DEST_DIR}/config.json"
        COMMAND ${CMAKE_COMMAND} -E copy ${CONFIG_FILE} ${DEST_DIR}
        DEPENDS ${CONFIG_FILE}
        COMMENT "Copying config.json to build directory"
)

# add a custom target that depends on the copy command
add_custom_target(copy-config ALL DEPENDS "${DEST_DIR}/config.json")

# create executable
add_executable(hello_world ${SRCS})

# link libraries
target_link_libraries(hello_world
        ${GLFW3_STATIC_LDFLAGS}
        ${FREETYPE2_STATIC_LDFLAGS}
        dl
        GL
        GLU
        X11
        pthread
        Xrandr
        Xi
        fmt::fmt
)

# custom output directory
set_target_properties(hello_world PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_dependencies(hello_world copy-config)

# CMakeLists.txt



# create custom target for cleaning
#add_custom_target(clean
#        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/build
#)