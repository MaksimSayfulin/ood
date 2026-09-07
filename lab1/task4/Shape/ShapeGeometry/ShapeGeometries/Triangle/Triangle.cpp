#include "Triangle.h"
#include <format>


shapes::Triangle::Triangle(double &&x1, double &&y1, double &&x2, double &&y2, double &&x3, double &&y3):
    m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2), m_x3(x3), m_y3(y3) {

}

void shapes::Triangle::Draw(gfx::ICanvas &canvas) const {
    canvas.MoveTo(m_x1, m_y1);
    canvas.LineTo(m_x2, m_y2);
    canvas.LineTo(m_x3, m_y3);
    canvas.LineTo(m_x1, m_y1);
}

void shapes::Triangle::Move(const double x, const double y) {
    m_x1 += x;
    m_y1 += y;
    m_x2 += x;
    m_y2 += y;
    m_x3 += x;
    m_y3 += y;
}

std::string shapes::Triangle::GetParameters() const {
    return std::format("{} {} {} {} {} {}", m_x1, m_y1, m_x2, m_y2, m_x3, m_y3);
}

std::string shapes::Triangle::GetShapeName() const {
    return "triangle";
}

shapes::IShapeGeometry *shapes::Triangle::Clone() const {
    return new Triangle(*this);
}
