#include "CanvasSvg.h"

#include <stdexcept>

gfx::CanvasSvg::CanvasSvg(std::ostream &stream): m_stream(stream) {
    if (!stream) {
        throw std::invalid_argument("Invalid stream");
    }

    m_stream << R"(<svg xmlns="http://www.w3.org/2000/svg" width="1920" height="1080">)" << std::endl;
}

gfx::CanvasSvg::~CanvasSvg() {
    m_stream << "</svg>";
}

void gfx::CanvasSvg::DrawEllipse(double cx, double cy, double rx, double ry) {
    const std::string ellipse = std::format("<ellipse cx=\"{}\" cy=\"{}\" rx=\"{}\" ry=\"{}\" fill=\"none\" stroke=\"{}\" />\n",
        cx, cy, rx, ry, m_color.GetColor());

    m_stream << ellipse;
}

void gfx::CanvasSvg::FillEllipse(double cx, double cy, double rx, double ry) {
    const std::string ellipse = std::format(
        "  <ellipse cx=\"{}\" cy=\"{}\" rx=\"{}\" ry=\"{}\" fill=\"{}\" stroke=\"none\" />\n",
        cx, cy, rx, ry, m_color.GetColor());

    m_stream << ellipse;
}

void gfx::CanvasSvg::DrawText(double left, double top, double fontSize, std::string text) {
        const std::string textSVG = std::format(
            "  <text x=\"{}\" y=\"{}\" font-size=\"{}\" fill=\"{}\">{}</text>\n",
            left, top, fontSize, m_color.GetColor(), text
        );
    m_stream << textSVG;
}

void gfx::CanvasSvg::LineTo(double x, double y) {
     const std::string line = std::format(
        "  <line x1=\"{}\" y1=\"{}\" x2=\"{}\" y2=\"{}\" stroke=\"{}\" stroke-width=\"2\" />\n",
        m_x, m_y, x, y, m_color.GetColor()
    );

    m_stream << line;
    TrackPath(x, y);
}

void gfx::CanvasSvg::Fill() {
    const auto& path = GetPath();
    if (path.size() < 3) {
        return;
    }

    std::string points;
    for (const auto& point : path) {
        points += std::format("{} {} ", point.x, point.y);
    }

    m_stream << std::format("  <polygon points=\"{}\" fill=\"{}\" stroke=\"none\" />\n",
        points, m_color.GetColor());
}
