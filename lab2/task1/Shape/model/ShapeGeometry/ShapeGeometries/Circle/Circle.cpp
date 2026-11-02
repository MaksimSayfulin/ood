#include "Circle.h"

#include <format>
#include <stdexcept>

shapes::Circle::Circle(double x, double y, double radius)
    : m_x(x)
    , m_y(y)
    , m_radius(radius) {
    if (m_radius < 0.0) {
        throw std::invalid_argument("Circle: radius must be non-negative");
    }
}
void shapes::Circle::Draw(gfx::ICanvas& canvas) const {
    canvas.DrawEllipse(m_x, m_y, m_radius, m_radius);
}

void shapes::Circle::Move(double x, double y) {
    m_x += x;
    m_y += y;
}

std::string shapes::Circle::GetParameters() const {
    return std::format("{} {} {}", m_x, m_y, m_radius);
}

std::string shapes::Circle::GetShapeName() const {
    return "circle";
}

shapes::IShapeGeometry *shapes::Circle::Clone() const {
    return new Circle(*this);
}

