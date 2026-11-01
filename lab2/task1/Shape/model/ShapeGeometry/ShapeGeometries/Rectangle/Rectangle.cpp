#include "Rectangle.h"

#include <format>
#include <stdexcept>

namespace {
    void Validate(const gfx::Rect& bounds) {
        if (!bounds.IsValid()) {
            throw std::invalid_argument("Rectangle: size must be non-negative");
        }
    }
}

shapes::Rectangle::Rectangle(const gfx::Rect &bounds) : m_bounds(bounds) {
    Validate(m_bounds);
}

void shapes::Rectangle::Draw(gfx::ICanvas& canvas) const {
    canvas.MoveTo(m_bounds.x, m_bounds.y);
    canvas.LineTo(m_bounds.Right(), m_bounds.y);
    canvas.LineTo(m_bounds.Right(), m_bounds.Bottom());
    canvas.LineTo(m_bounds.x, m_bounds.Bottom());
    canvas.LineTo(m_bounds.x, m_bounds.y);
    canvas.Fill();
}

void shapes::Rectangle::Move(const double x, const double y) {
    m_bounds.x += x;
    m_bounds.y += y;
}

gfx::Rect shapes::Rectangle::GetBounds() const {
    return m_bounds;
}

void shapes::Rectangle::SetBounds(const gfx::Rect& bounds) {
    Validate(bounds);
    m_bounds = bounds;
}

bool shapes::Rectangle::Contains(const double x, const double y) const {
    return m_bounds.Contains({x, y});
}

std::string shapes::Rectangle::GetParameters() const {
    return std::format("{} {} {} {}", m_bounds.x, m_bounds.y, m_bounds.width, m_bounds.height);
}

std::string shapes::Rectangle::GetShapeName() const {
    return "rectangle";
}

shapes::IShapeGeometry* shapes::Rectangle::Clone() const {
    return new Rectangle(*this);
}