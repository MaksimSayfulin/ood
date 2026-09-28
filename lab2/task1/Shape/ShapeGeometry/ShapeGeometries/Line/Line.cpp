#include "Line.h"

#include <format>

shapes::Line::Line(double &&x1, double &&y1, double &&x2, double &&y2): m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2) {

}

void shapes::Line::Draw(gfx::ICanvas& canvas) const {
    canvas.MoveTo(m_x1, m_y1);
    canvas.LineTo(m_x2, m_y2);
}

void shapes::Line::Move(const double x, const double y) {
    m_x1 += x;
    m_y1 += y;
    m_x2 += x;
    m_y2 += y;
}

std::string shapes::Line::GetParameters() const {
    return std::format("{} {} {} {}", m_x1, m_y1, m_x2, m_y2);
}

std::string shapes::Line::GetShapeName() const {
    return "line";
}

shapes::IShapeGeometry *shapes::Line::Clone() const {
    return new Line(*this);
}