#pragma once
#include <ostream>

#include "../ICanvas/ICanvas.h"

namespace gfx {
    class CanvasSvg : public ICanvas {
    public:
        explicit CanvasSvg(std::ostream &stream);
        ~CanvasSvg() override;
        void DrawEllipse(double cx, double cy, double rx, double ry) override;
        void DrawText(double left, double top, double fontSize, std::string text) override;
        void LineTo(double x, double y) override;
    private:
        std::ostream &m_stream;
    };
}