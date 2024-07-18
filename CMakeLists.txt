cmake_minimum_required(VERSION 3.10)
project(ssdb)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(
        lib/json
)

# include directories
include_directories(
        lib/glad/include
        lib/imgui
        lib/imgui/backends
        include
        ../glfw/glfw/install/include # This library is a fork of glfw on my local machine
        /usr/include/freetype2
        /usr/include/libpng16
)

# find packages
find_package(PkgConfig REQUIRED)
#pkg_check_modules(GLFW3 REQUIRED glfw3)
pkg_check_modules(FREETYPE2 REQUIRED freetype2)

find_package(fmt REQUIRED)


# add definitions
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

# source files
set(SRCS
        src/main.cpp
        lib/glad/src/glad.c
        lib/imgui/imgui.cpp
        lib/imgui/imgui_demo.cpp
        lib/imgui/imgui_draw.cpp
        lib/imgui/imgui_tables.cpp
        lib/imgui/imgui_widgets.cpp
        lib/imgui/backends/imgui_impl_glfw.cpp
        lib/imgui/backends/imgui_impl_opengl3.cpp
        include/FrameRateCalculator.hpp
        src/InputController.cpp
        include/InputController.hpp
#        Model.cpp
#        Model.hpp
#        View.cpp
#        View.hpp
        include/Text.hpp
        include/Model.hpp
        src/Store.cpp
        include/Store.hpp
        src/Subject.cpp
        include/Subject.hpp
        include/View.hpp
        src/SpreadsheetModel.cpp
        include/SpreadsheetModel.hpp
        src/Cell.cpp
        include/Cell.hpp
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
add_executable(ssdb ${SRCS})

# link libraries
target_link_libraries(ssdb
#        ${GLFW3_STATIC_LDFLAGS}
#        ../glfw/glfw/install/lib/libglfw3.a
        /home/zoravur/Documents/side-projects/spreadsheet-db/glfw/glfw/install/lib/libglfw3.a
        ${FREETYPE2_STATIC_LDFLAGS}
        dl
        GL
        GLU
        X11
        pthread
        Xrandr
        Xi
        fmt::fmt
        nlohmann_json::nlohmann_json
)

# custom output directory
set_target_properties(ssdb PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_dependencies(ssdb copy-config)

# CMakeLists.txt



# create custom target for cleaning
#add_custom_target(clean
#        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/build
#)