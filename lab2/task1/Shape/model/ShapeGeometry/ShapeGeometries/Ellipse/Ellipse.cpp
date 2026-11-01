#include "Ellipse.h"

#include <format>
#include <stdexcept>

namespace {
    void Validate(const gfx::Rect& bounds) {
        if (!bounds.IsValid()) {
            throw std::invalid_argument("Ellipse: size must be non-negative");
        }
    }
}

shapes::Ellipse::Ellipse(gfx::Rect bounds) : m_bounds(bounds) {
    Validate(m_bounds);
}

void shapes::Ellipse::Draw(gfx::ICanvas& canvas) const {
    const auto center = m_bounds.Center();
    const double rx = m_bounds.width / 2;
    const double ry = m_bounds.height / 2;
    canvas.FillEllipse(center.x, center.y, rx, ry);
    canvas.DrawEllipse(center.x, center.y, rx, ry);
}

void shapes::Ellipse::Move(const double x, const double y) {
    m_bounds.x += x;
    m_bounds.y += y;
}

gfx::Rect shapes::Ellipse::GetBounds() const {
    return m_bounds;
}

void shapes::Ellipse::SetBounds(const gfx::Rect& bounds) {
    Validate(bounds);
    m_bounds = bounds;
}

bool shapes::Ellipse::Contains(const double x, const double y) const {
    const auto center = m_bounds.Center();
    const double rx = m_bounds.width / 2;
    const double ry = m_bounds.height / 2;
    if (rx <= 0 || ry <= 0) {
        return false;
    }

    const double nx = (x - center.x) / rx;
    const double ny = (y - center.y) / ry;
    return nx * nx + ny * ny <= 1.0;
}

std::string shapes::Ellipse::GetParameters() const {
    return std::format("{} {} {} {}", m_bounds.x, m_bounds.y, m_bounds.width, m_bounds.height);
}

std::string shapes::Ellipse::GetShapeName() const {
    return "ellipse";
}

shapes::IShapeGeometry* shapes::Ellipse::Clone() const {
    return new Ellipse(*this);
}