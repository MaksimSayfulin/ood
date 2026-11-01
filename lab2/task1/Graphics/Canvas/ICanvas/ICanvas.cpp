#include "ICanvas.h"

#include <utility>

void gfx::ICanvas::SetColor(Color color) {
    m_color = std::move(color);
}

void gfx::ICanvas::MoveTo(const double x, const double y) {
    m_x = x;
    m_y = y;
    m_path.clear();
    m_path.push_back({x, y});
}

void gfx::ICanvas::TrackPath(const double x, const double y) {
    if (m_path.empty()) {
        m_path.push_back({m_x, m_y});
    }

    m_path.push_back({x, y});
    m_x = x;
    m_y = y;
}

std::vector<gfx::Point> gfx::ICanvas::TakePath() {
    return std::exchange(m_path, {});
}

const std::vector<gfx::Point> &gfx::ICanvas::GetPath() const {
    return m_path;
}