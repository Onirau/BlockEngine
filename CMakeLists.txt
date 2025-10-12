cmake_minimum_required(VERSION 3.15)
project(LemonEngine)

# For clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")
endif()

# Dependencies directory
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/dependencies")

# Add luau
if(EXISTS "${DEPS_DIR}/luau/CMakeLists.txt")
    add_subdirectory(${DEPS_DIR}/luau)
    set(LUAU_FOUND TRUE)
else()
    message(FATAL_ERROR "luau not found in dependencies")
endif()

# Add raylib
if(EXISTS "${DEPS_DIR}/raylib/CMakeLists.txt")
    add_subdirectory(${DEPS_DIR}/raylib)
    set(raylib_FOUND TRUE)
else()
    message(FATAL_ERROR "raylib not found in dependencies")
endif()

# Collect source files
file(GLOB_RECURSE COMMON_SOURCES 
    "src/core/*.cpp"
    "src/datatypes/*.cpp"
    "src/enums/*.cpp"
    "src/instances/*.cpp"
    "src/Global.cpp"
)

message(STATUS "Found source files:")
foreach(source ${COMMON_SOURCES})
    message(STATUS "  ${source}")
endforeach()

# Editor executable
file(GLOB EDITOR_SOURCES "src/editor/*.cpp")
add_executable(LemonEngineEditor ${EDITOR_SOURCES} ${COMMON_SOURCES})

# Runtime executable
file(GLOB RUNTIME_SOURCES "src/runtime/*.cpp")
add_executable(LemonEngineRuntime ${RUNTIME_SOURCES} ${COMMON_SOURCES})

# Include directories for both executables
foreach(TARGET LemonEngineEditor LemonEngineRuntime)
    target_include_directories(${TARGET} PRIVATE 
        ${DEPS_DIR}/luau/Ast/include
        ${DEPS_DIR}/luau/Config/include
        ${DEPS_DIR}/luau/VM/include
        ${DEPS_DIR}/luau/Compiler/include
        ${DEPS_DIR}/luau/Analysis/include
        ${CMAKE_SOURCE_DIR}
    )

    # Link libraries
    target_link_libraries(${TARGET} PRIVATE raylib Luau.VM Luau.Ast Luau.Compiler)

    # Platform-specific libraries
    if(WIN32)
        target_link_libraries(${TARGET} PRIVATE
            winmm
            gdi32
            opengl32
        )
    else()
        target_link_libraries(${TARGET} PRIVATE
            m
            dl
            pthread
            GL
            rt
            X11
        )
    endif()
endforeach()