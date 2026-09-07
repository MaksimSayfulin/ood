#pragma once
#include "../../Color/Color.h"

namespace gfx {
    class ICanvas {
    public:
        virtual ~ICanvas() = default;

        virtual void SetColor(Color color);

        void MoveTo(double x, double y);
        virtual void LineTo(double x, double y) = 0;
        virtual void DrawEllipse(double cx, double cy, double rx, double ry) = 0;
        virtual void DrawText(double left, double top, double fontSize, std::string text) = 0;

    protected:
        Color m_color = Color("#FFFFFF");
        double m_x = 0.0;
        double m_y = 0.0;
    };
}