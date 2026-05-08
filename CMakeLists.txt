cmake_minimum_required(VERSION 3.20)

project(MyGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML CONFIG REQUIRED COMPONENTS System Window Graphics Audio)

add_executable(MyGame
    src/main.cpp
    src/core/Game.cpp
    )

target_include_directories(MyGame PRIVATE include)

target_link_libraries(MyGame PRIVATE SFML::System SFML::Window SFML::Graphics SFML::Audio)

add_custom_command(TARGET MyGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:MyGame>/assets
)
