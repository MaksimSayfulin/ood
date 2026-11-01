#include "Line.h"

#include <algorithm>
#include <format>

namespace {
    constexpr double halfStroke = 3.0;
}

shapes::Line::Line(gfx::Rect bounds) : m_bounds(bounds) {
}

void shapes::Line::Draw(gfx::ICanvas& canvas) const {
    canvas.MoveTo(m_bounds.x, m_bounds.y);
    canvas.LineTo(m_bounds.Right(), m_bounds.Bottom());
}

void shapes::Line::Move(const double x, const double y) {
    m_bounds.x += x;
    m_bounds.y += y;
}

gfx::Rect shapes::Line::GetBounds() const {
    return m_bounds;
}

void shapes::Line::SetBounds(const gfx::Rect& bounds) {
    m_bounds = bounds;
}

bool shapes::Line::Contains(const double x, const double y) const {
    const double dx = m_bounds.width;
    const double dy = m_bounds.height;
    const double lengthSquared = dx * dx + dy * dy;

    const double t = lengthSquared == 0.0
        ? 0.0
        : std::clamp(((x - m_bounds.x) * dx + (y - m_bounds.y) * dy) / lengthSquared, 0.0, 1.0);

    const double px = m_bounds.x + t * dx - x;
    const double py = m_bounds.y + t * dy - y;
    return px * px + py * py <= halfStroke * halfStroke;
}

std::string shapes::Line::GetParameters() const {
    return std::format("{} {} {} {}", m_bounds.x, m_bounds.y, m_bounds.width, m_bounds.height);
}

std::string shapes::Line::GetShapeName() const {
    return "line";
}

shapes::IShapeGeometry* shapes::Line::Clone() const {
    return new Line(*this);
}