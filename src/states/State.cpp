// --- State.cpp ---

#include "State.h"
#include "../core/Game.h"
#include "../core/ResourceManager.h"
#include <cmath>

namespace game::states
{
    void State::setupButtonText(std::optional<sf::Text>& textObj, const sf::String& str, sf::Vector2f pos, int fontSize)
    {
        int adjustedSize = static_cast<int>(std::round(fontSize * GLOBAL_FONT_SCALE));

        // Teraz emplace przyjmie sf::String i zachowa 100% polskich znaków!
        textObj.emplace(game->mainFont, str, adjustedSize);
        textObj->setFillColor(sf::Color::White);

        sf::FloatRect bounds = textObj->getLocalBounds();
        textObj->setOrigin({
            std::round(bounds.size.x / 2.0f),
            std::round(bounds.position.y + bounds.size.y / 2.0f)
            });
        textObj->setPosition(pos);
    }

    void State::setupButton(const std::string& key, std::optional<sf::Sprite>& spr, sf::Vector2f pos, sf::Vector2f targetSize)
    {
        auto& rm = game::core::ResourceManager::get();
        if (rm.hasTexture(key))
        {
            spr.emplace(*rm.getTexture(key));
            sf::Vector2f originalSize(spr->getTexture().getSize());
            spr->setScale({ targetSize.x / originalSize.x, targetSize.y / originalSize.y });
            spr->setOrigin({ originalSize.x / 2.0f, originalSize.y / 2.0f });
            spr->setPosition(pos);
        }
    }

    void State::updateHover(std::optional<sf::Sprite>& btn, sf::Vector2f targetSize, sf::Vector2f mousePos, std::optional<sf::Text>* linkedText)
    {
        if (!btn) return;

        sf::Vector2f texSize(btn->getTexture().getSize());
        float baseScaleX = targetSize.x / texSize.x;
        float baseScaleY = targetSize.y / texSize.y;

        if (btn->getGlobalBounds().contains(mousePos)) {
            btn->setColor(sf::Color(255, 255, 255));
            btn->setScale({ baseScaleX * 1.1f, baseScaleY * 1.1f });

            if (linkedText && linkedText->has_value()) {
                (*linkedText)->setFillColor(sf::Color(255, 255, 255));
                (*linkedText)->setScale({ 1.1f, 1.1f });
            }
        }
        else {
            btn->setColor(sf::Color(210, 210, 210));
            btn->setScale({ baseScaleX, baseScaleY });

            if (linkedText && linkedText->has_value()) {
                (*linkedText)->setFillColor(sf::Color(210, 210, 210));
                (*linkedText)->setScale({ 1.0f, 1.0f });
            }
        }
    }
}