cmake_minimum_required(VERSION 3.20)

project(FruitalCombat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Zdefiniowanie ?cie?ki do folderu ze skompilowanym FFmpeg
set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/vendor/ffmpeg")

# ?cie?ka do SFML (Wa?ne: wskazujemy podfolder z plikami konfiguracji .cmake)
set(SFML_DIR "${CMAKE_SOURCE_DIR}/vendor/SFML/lib/cmake/SFML")


# --- 1. ZALE?NO?CI ZEWN?TRZNE (SFML + FFmpeg) ---
find_package(SFML CONFIG REQUIRED COMPONENTS System Window Graphics Audio)


# --- 2. PLIKI ?RÓD?OWE GRY ---
add_executable(FruitalCombat
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
    src/components/ShotgunAbility.cpp
    src/utils/VideoPlayer.h
 "src/states/CharacterSelectState.h" "src/states/CharacterSelectState.cpp")


# --- 3. NAG?ÓWKI I LINKOWANIE ---
# Dodajemy foldery nag?ówkowe: Twój wewn?trzny 'include' oraz 'include' z FFmpeg
target_include_directories(FruitalCombat PRIVATE 
    include 
    "${FFMPEG_DIR}/include"
)

# Wskazujemy linkerowi, gdzie le?? pliki .lib z FFmpeg
target_link_directories(FruitalCombat PRIVATE "${FFMPEG_DIR}/lib")

# Linkujemy biblioteki SFML oraz 4 kluczowe modu?y FFmpeg
target_link_libraries(FruitalCombat PRIVATE 
    # SFML
    SFML::System 
    SFML::Window 
    SFML::Graphics 
    SFML::Audio
    # FFmpeg
    avformat
    avcodec
    avutil
    swscale
)


# --- 4. OPERACJE PO KOMPILACJI (POST-BUILD) ---

# Kopiowanie folderu z zasobami (assets)
add_custom_command(TARGET FruitalCombat POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:FruitalCombat>/assets"
    COMMENT "Kopiowanie folderu assets..."
)

# Automatyczne kopiowanie bibliotek DLL z FFmpeg obok pliku .exe (tylko na Windowsie)
if(WIN32)
    file(GLOB FFMPEG_DLLS "${FFMPEG_DIR}/bin/*.dll")
    
    add_custom_command(TARGET FruitalCombat POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${FFMPEG_DLLS}
            "$<TARGET_FILE_DIR:FruitalCombat>"
        COMMENT "Kopiowanie bibliotek DLL FFmpeg do folderu wyj?ciowego..."
    )
endif()