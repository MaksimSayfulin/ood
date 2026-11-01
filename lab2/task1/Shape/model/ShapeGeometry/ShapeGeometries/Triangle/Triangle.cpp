#include "Triangle.h"

#include <format>
#include <stdexcept>

namespace {
    double Cross(const gfx::Point& edgeStart, const gfx::Point& edgeEnd, const gfx::Point& point) {
        return (edgeEnd.x - edgeStart.x) * (point.y - edgeStart.y)
             - (edgeEnd.y - edgeStart.y) * (point.x - edgeStart.x);
    }
}

shapes::Triangle::Triangle(gfx::Rect bounds) : m_bounds(bounds) {
    if (!bounds.IsValid()) {
        throw std::invalid_argument("Triangle: size must be non-negative");
    }
}

std::array<gfx::Point, 3> shapes::Triangle::GetPoints() const {
    return {{
        {m_bounds.x, m_bounds.Bottom()},
        {m_bounds.x + m_bounds.width / 2, m_bounds.y},
        {m_bounds.Right(), m_bounds.Bottom()}
    }};
}

void shapes::Triangle::Draw(gfx::ICanvas& canvas) const {
    const auto points = GetPoints();

    canvas.MoveTo(points[0].x, points[0].y);
    for (std::size_t i = 1; i < points.size(); ++i) {
        canvas.LineTo(points[i].x, points[i].y);
    }
    canvas.LineTo(points[0].x, points[0].y);
    canvas.Fill();
}

void shapes::Triangle::Move(const double x, const double y) {
    m_bounds.x += x;
    m_bounds.y += y;
}

gfx::Rect shapes::Triangle::GetBounds() const {
    return m_bounds;
}

void shapes::Triangle::SetBounds(const gfx::Rect& bounds) {
    if (!bounds.IsValid()) {
        throw std::invalid_argument("Triangle: size must be non-negative");
    }
    m_bounds = bounds;
}

bool shapes::Triangle::Contains(const double x, const double y) const {
    const auto points = GetPoints();
    const gfx::Point point{x, y};

    const double d1 = Cross(points[0], points[1], point);
    const double d2 = Cross(points[1], points[2], point);
    const double d3 = Cross(points[2], points[0], point);

    const bool hasNegative = d1 < 0 || d2 < 0 || d3 < 0;
    const bool hasPositive = d1 > 0 || d2 > 0 || d3 > 0;
    return !(hasNegative && hasPositive);
}

std::string shapes::Triangle::GetParameters() const {
    return std::format("{} {} {} {}", m_bounds.x, m_bounds.y, m_bounds.width, m_bounds.height);
}

std::string shapes::Triangle::GetShapeName() const {
    return "triangle";
}

shapes::IShapeGeometry* shapes::Triangle::Clone() const {
    return new Triangle(*this);
}