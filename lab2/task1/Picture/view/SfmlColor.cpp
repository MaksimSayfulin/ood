#include "SfmlColor.h"

#include <cctype>
#include <cstdint>
#include <string>

namespace {
    std::uint8_t ParseChannel(const std::string& digits, const std::size_t offset) {
        const auto value = static_cast<std::uint8_t>(std::stoul(digits.substr(offset, 2), nullptr, 16));
        return value;
    }
}

sf::Color gfx::ToSfmlColor(const gfx::Color& color) {
    std::string digits = color.GetColor();
    if (!digits.empty() && digits.front() == '#') {
        digits.erase(digits.begin());
    }

    if (digits.size() != 6 && digits.size() != 8) {
        return sf::Color::White;
    }

    const auto red = ParseChannel(digits, 0);
    const auto green = ParseChannel(digits, 2);
    const auto blue = ParseChannel(digits, 4);
    const auto alpha = digits.size() == 8 ? ParseChannel(digits, 6) : std::uint8_t{255};

    return {red, green, blue, alpha};
}
