#include "Color.h"
#include <regex>
#include <utility>

namespace {
    bool IsValidHexColor(const std::string& color) {
        static const std::regex hexColorRegex("^#[0-9a-fA-F]{6}$");

        return std::regex_match(color, hexColorRegex);
    }
}

gfx::Color::Color(std::string color): m_color(std::move(color)) {
    if (!IsValidHexColor(m_color)) {
        throw std::invalid_argument("Invalid color");
    }
}

std::string gfx::Color::GetColor() {
    return m_color;
}
