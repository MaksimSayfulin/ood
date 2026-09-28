#include "Text.h"

#include <format>
#include <stdexcept>

shapes::Text::Text(double &&left, double &&top, double &&scale, std::string &&text):
    m_left(left), m_top(top), m_scale(scale), m_text(text) {
    if (m_scale < 0.0) {
        throw std::invalid_argument("Scale must be non-negative");
    }
}

void shapes::Text::Draw(gfx::ICanvas &canvas) const {
    canvas.DrawText(m_left, m_top, m_scale, m_text);
}

void shapes::Text::Move(const double x, const double y) {
    m_left += x;
    m_top += y;
}

std::string shapes::Text::GetParameters() const {
    return std::format("{} {} {} {}", m_left, m_top, m_scale, m_text);
}

std::string shapes::Text::GetShapeName() const {
    return "text";
}

shapes::IShapeGeometry *shapes::Text::Clone() const {
    return new Text(*this);
}