#include "CanvasSvg.h"

#include <fstream>

gfx::CanvasSvg::CanvasSvg(std::ofstream &stream): m_stream(stream) {
    if (!stream.is_open()) {
        throw std::invalid_argument("Cannot open stream");
    }

    m_stream << R"(<svg xmlns="http://www.w3.org/2000/svg" width="1920" height="1080">)" << std::endl;
}

gfx::CanvasSvg::~CanvasSvg() {
    m_stream << "</svg>";
    m_stream.close();
}

void gfx::CanvasSvg::DrawEllipse(double cx, double cy, double rx, double ry) {
    const std::string ellipse = std::format("<ellipse cx=\"{}\" cy=\"{}\" rx=\"{}\" ry=\"{}\" fill=\"none\" stroke=\"{}\" />\n",
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
    MoveTo(x, y);
}
