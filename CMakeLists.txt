cmake_minimum_required(VERSION 3.20)

project(MyGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML CONFIG REQUIRED COMPONENTS System Window Graphics Audio)

add_executable(MyGame
    src/main.cpp
    src/core/Config.h
    src/core/Game.h
    src/core/Game.cpp
    src/core/StateMachine.h
    src/core/StateMachine.cpp
    src/core/ArenaContext.h
    src/core/State.h
    src/states/IntroState.h
    src/states/IntroState.cpp
    src/states/MenuState.h
    src/states/MenuState.cpp
    src/states/PlayingState.h
    src/states/PlayingState.cpp
    src/states/SettingsState.h
    src/states/SettingsState.cpp
    src/factories/FruitFactory.h
    src/factories/FruitFactory.cpp
    src/entities/EntityTypes.h
    src/entities/Entity.h
    src/entities/player/Player.h
    src/entities/player/Player.cpp
    src/components/Ability.h
    src/components/DashAbility.h
    src/components/DashAbility.cpp
    src/components/ShootAbility.h
    src/components/ShootAbility.cpp
    src/components/Bullet.h
    src/components/Bullet.cpp
)

target_include_directories(MyGame PRIVATE include)

target_link_libraries(MyGame PRIVATE SFML::System SFML::Window SFML::Graphics SFML::Audio)

# Kopiowanie folderu z zasobami (assets) po udanej kompilacji
add_custom_command(TARGET MyGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:MyGame>/assets
)