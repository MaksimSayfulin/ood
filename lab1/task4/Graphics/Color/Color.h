#pragma once
#include <string>

namespace gfx {
    class Color {
    public:
        explicit Color(std::string color);
        std::string GetColor();

    private:
        std::string m_color;
    };
}