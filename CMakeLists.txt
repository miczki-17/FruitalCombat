cmake_minimum_required(VERSION 3.20)

project(FruitalCombat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. ZALEØNOåCI ZEWN TRZNE (SFML) ---
# UsuniÍto sztywnπ úcieøkÍ SFML_DIR. Teraz CMake szuka biblioteki uniwersalnie.
find_package(SFML CONFIG REQUIRED COMPONENTS System Window Graphics Audio)


# --- 2. PLIKI èR”D£OWE GRY ---
add_executable(FruitalCombat
    src/main.cpp

    # --- ABILITIES ---
    src/abilities/Ability.h
    src/abilities/AcidPoolUltimate.h
    src/abilities/AcidPoolUltimate.cpp
    src/abilities/AcidSquirtAbility.h
    src/abilities/AcidSquirtAbility.cpp
    src/abilities/CiderDashAbility.h
    src/abilities/CiderDashAbility.cpp
    src/abilities/DashAbility.h
    src/abilities/DashAbility.cpp
    src/abilities/MultiShootAbility.h
    src/abilities/MultiShootAbility.cpp
    src/abilities/PoisonExplosionAbility.h
    src/abilities/PoisonExplosionAbility.cpp
    src/abilities/RindRollAbility.h
    src/abilities/RindRollAbility.cpp
    src/abilities/ShootAbility.h
    src/abilities/ShootAbility.cpp
    src/abilities/ShotgunAbility.cpp
    src/abilities/WindupBruisierAbility.h
    src/abilities/WindupBruisierAbility.cpp

    # --- ENTITIES ---
    src/entities/EntityTypes.h
    src/entities/Entity.h

    # --- CORE ---
    src/core/Config.h
    src/core/Game.h
    src/core/Game.cpp
    src/core/StateMachine.h
    src/core/StateMachine.cpp
    src/core/ArenaContext.h
    src/core/ResourceManager.h
    src/core/ResourceManager.cpp
    src/core/PlayerProfile.h
    src/core/GameWorld.h
    src/core/GameWorld.cpp
    src/core/AudioManager.h
    src/core/AudioManager.cpp
    src/core/AssetGroup.h
    src/core/LocalizationManager.h
    src/core/LocalizationManager.cpp
    src/genetics/DNA.h

    # --- COMPONENTS ---
    src/components/Component.h
    src/components/MovementComponent.h
    src/components/MovementComponent.cpp
    src/components/ColliderComponent.h
    src/components/ColliderComponent.cpp
    src/components/SpriteComponent.h
    src/components/SpriteComponent.cpp
    src/components/AbilityComponent.h
    src/components/AbilityComponent.cpp
    src/components/StatsComponent.h
    src/components/StatsComponent.cpp
    src/components/DNAComponent.h
    src/components/DNAComponent.cpp
    src/components/AiInputComponent.h
    src/components/AiInputComponent.cpp
    src/components/PlayerInputComponent.h
    src/components/PlayerInputComponent.cpp
    src/components/ProjectileComponent.h
    src/components/ProjectileComponent.cpp
    src/components/TransformComponent.h
    src/components/TransformComponent.cpp
    src/components/TextComponent.h
    src/components/TextComponent.cpp
    src/components/PopAnimationComponent.h
    src/components/PopAnimationComponent.cpp
    src/components/ParticleComponent.h
    src/components/ParticleComponent.cpp
    src/components/JuiceComponent.h
    src/components/AoEComponent.h
    src/components/AoEComponent.cpp
    src/components/AnimationComponent.h
    src/components/AnimationComponent.cpp
    src/components/TraitDisplayComponent.h
    src/components/TraitDisplayComponent.cpp
    src/components/DeathRattleComponent.h
    src/components/SplitOnDeathComponent.h
    src/components/MedkitComponent.h
    src/components/PickupComponent.h
    src/components/PickupComponent.cpp
    src/components/ManaPouchComponent.h
    src/components/LifespanComponent.h
    src/components/LifespanComponent.cpp

    # --- FACTORIES & SYSTEMS ---
    src/factories/MutantFactory.h
    src/factories/MutantFactory.cpp
    src/factories/FruitFactory.h
    src/factories/FruitFactory.cpp
    src/systems/EvolutionManager.h
    src/systems/EvolutionManager.cpp
    src/systems/CollisionSystem.h
    src/systems/CollisionSystem.cpp
    src/systems/CombatSystem.h
    src/systems/CombatSystem.cpp
    src/systems/ParticleSystem.h
    src/systems/ParticleSystem.cpp
    src/systems/RenderSystem.h
    src/systems/RenderSystem.cpp
    src/systems/MapHazardSystem.h
    src/systems/MapHazardSystem.cpp
    src/systems/ItemSpawnSystem.h
    src/systems/ItemSpawnSystem.cpp

    # --- STATES ---
    src/states/State.h
    src/states/State.cpp
    src/states/ShopState.h
    src/states/ShopState.cpp
    src/states/DeathState.h
    src/states/DeathState.cpp
    src/states/LobbyState.h
    src/states/LobbyState.cpp
    src/states/ShopInGameState.h
    src/states/ShopInGameState.cpp
    src/states/IntroState.h
    src/states/IntroState.cpp
    src/states/MenuState.h
    src/states/MenuState.cpp
    src/states/PlayingState.h
    src/states/PlayingState.cpp
    src/states/SettingsState.h
    src/states/SettingsState.cpp
    src/states/BestiariusState.h
    src/states/BestiariusState.cpp
    src/states/CharacterSelectState.h
    src/states/CharacterSelectState.cpp
    src/states/PauseState.h
    src/states/PauseState.cpp
    src/states/MapSelectState.h
    src/states/MapSelectState.cpp

    # --- UTILS ---
    src/utils/MathUtils.h
    src/utils/RoundedRectangleShape.h
)


# --- 3. NAG£”WKI I LINKOWANIE ---
target_include_directories(FruitalCombat PRIVATE include)

target_link_libraries(FruitalCombat PRIVATE 
    SFML::System 
    SFML::Window 
    SFML::Graphics 
    SFML::Audio
)


# --- 4. OPERACJE PO KOMPILACJI (POST-BUILD) ---
add_custom_command(TARGET FruitalCombat POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:FruitalCombat>/assets"
    COMMENT "Kopiowanie folderu assets..."
)