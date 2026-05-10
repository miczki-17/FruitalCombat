cmake_minimum_required(VERSION 3.20)

project(MyGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML CONFIG REQUIRED COMPONENTS System Window Graphics Audio)

add_executable(MyGame
    src/main.cpp
    src/core/Game.cpp
     "src/states/PlayingState.h" "src/states/PlayingState.cpp" "src/entities/player/Player.h" "src/states/MenuState.cpp" "src/states/IntroState.h" "src/states/IntroState.cpp" "src/core/StateMachine.h" "src/core/StateMachine.cpp" "src/states/SettingsState.h" "src/states/SettingsState.cpp" "src/entities/EntityTypes.h" "src/entities/Entity.h" "src/entities/player/Player.cpp" "src/entities/skills/Skill.h" "src/entities/skills/DashSkill.h" "src/entities/skills/SeedShootSkill.h")

target_include_directories(MyGame PRIVATE include)

target_link_libraries(MyGame PRIVATE SFML::System SFML::Window SFML::Graphics SFML::Audio)

add_custom_command(TARGET MyGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:MyGame>/assets
)
