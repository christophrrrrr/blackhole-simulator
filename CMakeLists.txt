cmake_minimum_required(VERSION 3.10)
project(blackhole-simulator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# source files
set(SOURCES
    src/main.c
    src/math_utils.c
    src/camera.c
    src/physics.c
    src/grid.c
    src/shaders.c
    src/renderer.c
    src/callbacks.c
)

# include directories
include_directories(include)

# platform-specific configuration
if(APPLE)
    # macOS: use homebrew paths and frameworks
    set(CMAKE_PREFIX_PATH "/opt/homebrew" ${CMAKE_PREFIX_PATH})
    find_library(GLFW_LIB NAMES glfw PATHS /opt/homebrew/lib)
    find_path(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h PATHS /opt/homebrew/include)
    
    include_directories(${GLFW_INCLUDE_DIR})
    
    # frameworks for macOS
    find_library(OPENGL_FRAMEWORK OpenGL)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    
    set(PLATFORM_LIBS
        ${GLFW_LIB}
        ${OPENGL_FRAMEWORK}
        ${COCOA_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
        pthread
    )
else()
    # Linux: use system packages
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED glfw3)
    
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    
    include_directories(${GLFW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
    
    set(PLATFORM_LIBS
        ${GLFW_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        m
        dl
        pthread
    )
endif()

# create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# link libraries
target_link_libraries(${PROJECT_NAME} ${PLATFORM_LIBS})

# compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -O2)

# on macOS, silence deprecation warnings
if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GL_SILENCE_DEPRECATION)
endif()

