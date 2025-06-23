cmake_minimum_required(VERSION 3.16)

project(alice2 VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(OpenGL REQUIRED)

# Use GLEW from ALICE_PLATFORM
set(ALICE_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ALICE_PLATFORM")
set(GLEW_INCLUDE_DIRS "${ALICE_PLATFORM_DIR}/src/glew/include")
set(GLEW_LIBRARIES "${ALICE_PLATFORM_DIR}/src/glew/lib/glew32.lib")

# Check if GLEW files exist
if(NOT EXISTS ${GLEW_INCLUDE_DIRS}/GL/glew.h)
    message(FATAL_ERROR "GLEW headers not found at ${GLEW_INCLUDE_DIRS}")
endif()
if(NOT EXISTS ${GLEW_LIBRARIES})
    message(FATAL_ERROR "GLEW library not found at ${GLEW_LIBRARIES}")
endif()

# Use GLFW from local depends directory
set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/depends/glfw")
set(GLFW_INCLUDE_DIRS "${GLFW_DIR}/include")

# Check if GLFW files exist
if(NOT EXISTS ${GLFW_INCLUDE_DIRS}/GLFW/glfw3.h)
    message(FATAL_ERROR "GLFW headers not found at ${GLFW_INCLUDE_DIRS}")
endif()

# Build GLFW as a subdirectory
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${GLFW_DIR} glfw_build)

message(STATUS "Using GLFW from local depends directory")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
)

# Source files
set(ALICE2_SOURCES
    # Core
    src/core/Application.cpp
    src/core/Camera.cpp
    src/core/Renderer.cpp
    src/core/Scene.cpp
    src/core/Transform.cpp
    
    # Objects
    src/objects/SceneObject.cpp
    src/objects/ZSpaceObject.cpp
    src/objects/PrimitiveObject.cpp
    
    # Input
    src/input/InputManager.cpp
    src/input/CameraController.cpp

    # Utils
    src/utils/Math.cpp

    # Sketches
    src/sketches/SketchManager.cpp

    # User sketches
    userSrc/sketch_base.cpp

    # Main
    src/main.cpp
)

# Header files
set(ALICE2_HEADERS
    # Main include
    include/alice2.h
    
    # Core
    src/core/Application.h
    src/core/Camera.h
    src/core/Renderer.h
    src/core/Scene.h
    src/core/Transform.h
    
    # Objects
    src/objects/SceneObject.h
    src/objects/ZSpaceObject.h
    src/objects/PrimitiveObject.h
    
    # Input
    src/input/InputManager.h
    src/input/CameraController.h
    
    # Sketches
    src/sketches/ISketch.h
    src/sketches/SketchManager.h
    
    # Utils
    src/utils/Math.h
    src/utils/OpenGL.h
)

# Create the executable
add_executable(alice2 ${ALICE2_SOURCES} ${ALICE2_HEADERS})

# Link libraries
target_link_libraries(alice2
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    glfw
)

# Compiler-specific options
if(MSVC)
    target_compile_options(alice2 PRIVATE /W4)
    target_compile_definitions(alice2 PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(alice2 PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_compile_definitions(alice2 PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    
    # Copy DLLs to output directory for Windows
    if(GLEW_FOUND)
        get_filename_component(GLEW_DLL_DIR ${GLEW_LIBRARIES} DIRECTORY)
        file(GLOB GLEW_DLLS "${GLEW_DLL_DIR}/../bin/*.dll")
        foreach(dll ${GLEW_DLLS})
            add_custom_command(TARGET alice2 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${dll} $<TARGET_FILE_DIR:alice2>)
        endforeach()
    endif()
endif()

# Install targets
install(TARGETS alice2
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install user source directory
install(DIRECTORY userSrc/
    DESTINATION userSrc
    FILES_MATCHING PATTERN "*.cpp" PATTERN "*.h"
)

# Print configuration summary
message(STATUS "alice2 Configuration Summary:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  OpenGL: ${OPENGL_LIBRARIES}")
message(STATUS "  GLEW: ${GLEW_LIBRARIES}")
message(STATUS "  GLFW: glfw (built from source)")

# Optional: Create a build script for convenience
if(WIN32)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build.bat.in
        ${CMAKE_CURRENT_BINARY_DIR}/build.bat
        @ONLY
    )
else()
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build.sh.in
        ${CMAKE_CURRENT_BINARY_DIR}/build.sh
        @ONLY
    )
endif()
