#pragma once
#include <string>

namespace gfx {
    class Color {
    public:
        explicit Color(std::string color);
        std::string GetColor() const;

        bool operator==(const Color & color) const = default;

    private:
        std::string m_color;
    };
}