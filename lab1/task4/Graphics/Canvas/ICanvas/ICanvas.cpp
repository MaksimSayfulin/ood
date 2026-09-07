#include "ICanvas.h"
#include <utility>

void gfx::ICanvas::SetColor(Color color) {
    m_color = std::move(color);
}

void gfx::ICanvas::MoveTo(const double x, const double y) {
    m_x = x;
    m_y = y;
}

