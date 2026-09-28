#include "Rectangle.h"

#include <format>
#include <stdexcept>

shapes::Rectangle::Rectangle(double &&left, double &&top, double &&width, double &&height):
    m_top(top), m_left(left), m_height(height), m_width(width) {
    if (m_height < 0.0) {
        throw std::invalid_argument("height must be greater than zero");
    }

    if (m_width < 0.0) {
        throw std::invalid_argument("width must be greater than zero");
    }
}

void shapes::Rectangle::Draw(gfx::ICanvas &canvas) const {
    canvas.MoveTo(m_left, m_top);
    canvas.LineTo(m_left + m_width, m_top);
    canvas.LineTo(m_left + m_width, m_top + m_height);
    canvas.LineTo(m_left, m_top + m_height);
    canvas.LineTo(m_left, m_top);
}

void shapes::Rectangle::Move(const double x, const double y) {
    m_left += x;
    m_top += y;
}

std::string shapes::Rectangle::GetParameters() const {
    return std::format("{} {} {} {}", m_left, m_top, m_width, m_height);
}

std::string shapes::Rectangle::GetShapeName() const {
    return "rectangle";
}

shapes::IShapeGeometry *shapes::Rectangle::Clone() const {
    return new Rectangle(*this);
}