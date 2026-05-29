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
    "src/states/State.h"
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
    
    
    "src/abilities/Ability.h"
    "src/abilities/DashAbility.h"
    "src/abilities/DashAbility.cpp"
    "src/abilities/ShootAbility.h"
    "src/abilities/ShootAbility.cpp"
    "src/projectiles/Bullet.h"
    
    "src/abilities/ShotgunAbility.cpp"
    src/utils/VideoPlayer.h
    "src/utils/AnimationController.h"
    "src/utils/AnimationController.cpp"
    src/states/CharacterSelectState.h
    src/states/CharacterSelectState.cpp
    src/states/PauseState
    src/states/PauseState.cpp
    src/states/MapSelectState.h
    src/states/MapSelectState.cpp
    src/components/ProjectileComponent.h
    src/components/ProjectileComponent.cpp
 "src/abilities/AcidSquirtAbility.h" "src/abilities/AcidSquirtAbility.cpp" "src/abilities/RindRollAbility.h" "src/abilities/RindRollAbility.cpp" "src/effects/AcidSplash.h" "src/effects/AcidSplash.cpp" "src/core/ResourceManager.h" "src/components/Component.h" "src/components/MovementComponent.h" "src/components/ColliderComponent.h" "src/components/SpriteComponent.h" "src/components/AbilityComponent.h" "src/components/StatsComponent.h" "src/genetics/DNA.h" "src/components/DNAComponent.h" "src/components/AiInputComponent.h" "src/factories/MutantFactory.h" "src/factories/MutantFactory.cpp" "src/systems/EvolutionManager.h" "src/systems/EvolutionManager.cpp" "src/effects/FloatingText.h" "src/entities/JuiceDrop.h" "src/states/ShopState.h" "src/states/ShopState.cpp" "src/components/StatsComponent.cpp" "src/abilities/AcidPoolUltimate.h" "src/abilities/AcidPoolUltimate.cpp" "src/components/AbilityComponent.cpp" "src/components/MovementComponent.cpp" "src/components/AiInputComponent.cpp" "src/components/ColliderComponent.cpp" "src/components/SpriteComponent.cpp" "src/components/DNAComponent.cpp" "src/core/ResourceManager.cpp" "src/utils/MathUtils.h" "src/components/PlayerInputComponent.h" "src/components/PlayerInputComponent.cpp" "src/components/ProjectileComponent.h" "src/components/ProjectileComponent.cpp" "src/systems/CollisionSystem.h" "src/systems/CollisionSystem.cpp" "src/systems/CombatSystem.h" "src/systems/CombatSystem.cpp" "src/systems/ParticleSystem.h" "src/systems/ParticleSystem.cpp" "src/systems/RenderSystem.h" "src/systems/RenderSystem.cpp" "src/core/PlayerProfile.h" "src/systems/MapHazardSystem.h" "src/systems/MapHazardSystem.cpp" "src/core/GameWorld.h" "src/core/GameWorld.cpp" "src/core/AudioManager.h" "src/core/AudioManager.cpp" "src/core/AssetsGroup.h")


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