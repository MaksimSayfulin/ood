#include "SfmlCanvas.h"

#include "SfmlColor.h"

#include <cctype>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <string>
#include <utility>

namespace {
    constexpr std::size_t ellipseSegments = 64;
    constexpr float strokeThickness = 2.f;

    sf::Vertex Vertex(double x, double y, const sf::Color& color) {
        return {{float(x), float(y)}, color};
    }
}

gfx::SfmlCanvas::SfmlCanvas(sf::RenderTarget& target, const sf::Font* font)
    : m_target(target)
    , m_font(font) {
}

void gfx::SfmlCanvas::SetColor(Color color) {
    ICanvas::SetColor(std::move(color));
}

void gfx::SfmlCanvas::LineTo(const double x, const double y) {
    const auto color = CurrentColor();

    const float dx = static_cast<float>(x - m_x);
    const float dy = static_cast<float>(y - m_y);
    const float length = std::hypot(dx, dy);
    if (length == 0.f) {
        return;
    }

    sf::RectangleShape stroke({length, strokeThickness});
    stroke.setOrigin({0.f, strokeThickness / 2});
    stroke.setPosition({static_cast<float>(m_x), static_cast<float>(m_y)});
    stroke.setRotation(sf::radians(std::atan2(dy, dx)));
    stroke.setFillColor(color);

    m_target.draw(stroke, m_states);
    TrackPath(x, y);
}

void gfx::SfmlCanvas::Fill() {
    const auto path = TakePath();
    if (path.size() < 3) {
        return;
    }

    sf::ConvexShape polygon;
    polygon.setPointCount(path.size());
    for (std::size_t i = 0; i < path.size(); ++i) {
        polygon.setPoint(i, {static_cast<float>(path[i].x), static_cast<float>(path[i].y)});
    }
    polygon.setFillColor(CurrentColor());

    m_target.draw(polygon, m_states);
}

void gfx::SfmlCanvas::FillEllipse(const double cx, const double cy, const double rx, const double ry) {
    sf::ConvexShape ellipse;
    ellipse.setPointCount(ellipseSegments);
    for (std::size_t i = 0; i < ellipseSegments; ++i) {
        const auto angle = 2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(ellipseSegments);
        ellipse.setPoint(i, {static_cast<float>(cx + rx * std::cos(angle)),
                             static_cast<float>(cy + ry * std::sin(angle))});
    }
    ellipse.setFillColor(CurrentColor());

    m_target.draw(ellipse, m_states);
}

void gfx::SfmlCanvas::DrawEllipse(const double cx, const double cy, const double rx, const double ry) {
    const auto color = CurrentColor();

    DrawEllipseOutline(cx, cy, rx, ry, color);
    if (rx > 1.0 && ry > 1.0) {
        DrawEllipseOutline(cx, cy, rx - 1.0, ry - 1.0, color);
    }
}

void gfx::SfmlCanvas::DrawEllipseOutline(const double cx, const double cy,
                                        const double rx, const double ry, const sf::Color& color) {
    sf::VertexArray outline(sf::PrimitiveType::LineStrip, ellipseSegments + 1);
    for (std::size_t i = 0; i <= ellipseSegments; ++i) {
        const auto angle = 2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(ellipseSegments);
        outline[i] = Vertex(cx + rx * std::cos(angle), cy + ry * std::sin(angle), color);
    }

    m_target.draw(outline, m_states);
}

void gfx::SfmlCanvas::DrawText(const double left, const double top, const double fontSize, std::string text) {
    if (m_font == nullptr) {
        return;
    }

    sf::Text label(*m_font, std::move(text), static_cast<unsigned int>(fontSize));
    label.setFillColor(CurrentColor());
    label.setPosition({float(left), float(top - fontSize)});

    m_target.draw(label, m_states);
}

void gfx::SfmlCanvas::SetOverrideColor(std::optional<sf::Color> color) {
    m_overrideColor = color;
}

void gfx::SfmlCanvas::SetOffset(const sf::Vector2f& offset) {
    m_states.transform = sf::Transform{};
    m_states.transform.translate(offset);
}

sf::Color gfx::SfmlCanvas::CurrentColor() const {
    return m_overrideColor.value_or(ToSfmlColor(m_color));
}