#pragma once

#include <SFML/Graphics.hpp>

#include "../../Graphics/Color/Color.h"

namespace gfx {
    [[nodiscard]] sf::Color ToSfmlColor(const Color& color);
}
