cmake_minimum_required(VERSION 3.28)
project(CHIP-8_INTERPRETER LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)

FetchContent_MakeAvailable(ImGui)
set(IMGUI_DIR ${imgui_SOURCE_DIR})

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)

FetchContent_Declare(imgui-sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v3.0
)

set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SFML imgui-sfml)

add_executable(main 
    src/main.cpp 
    src/chip8.cpp 
    src/opcodes.cpp 
    src/disassembler.cpp
)

target_compile_features(main PRIVATE cxx_std_17)
target_include_directories(main PRIVATE include)

target_link_libraries(main PRIVATE 
    SFML::Graphics 
    SFML::Audio         
    SFML::Window          
    SFML::System
    ImGui-SFML::ImGui-SFML
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
