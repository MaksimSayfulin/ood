#pragma once

#include <optional>
#include <SFML/Graphics.hpp>

#include "../../Graphics/Canvas/ICanvas/ICanvas.h"

namespace gfx {
    class SfmlCanvas : public ICanvas {
    public:
        explicit SfmlCanvas(sf::RenderTarget& target, const sf::Font* font = nullptr);

        void SetColor(Color color) override;
        void LineTo(double x, double y) override;
        void Fill() override;
        void FillEllipse(double cx, double cy, double rx, double ry) override;
        void DrawEllipse(double cx, double cy, double rx, double ry) override;
        void DrawText(double left, double top, double fontSize, std::string text) override;

        void SetOverrideColor(std::optional<sf::Color> color);

        void SetOffset(const sf::Vector2f& offset);

    private:
        [[nodiscard]] sf::Color CurrentColor() const;
        void DrawEllipseOutline(double cx, double cy, double rx, double ry, const sf::Color& color);

        sf::RenderTarget& m_target;
        const sf::Font* m_font;
        std::optional<sf::Color> m_overrideColor;
        sf::RenderStates m_states;
    };
}