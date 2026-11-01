#pragma once

#include <string>
#include <vector>

#include "../../Color/Color.h"
#include "../../Geometry/Rect.h"

namespace gfx {
    class ICanvas {
    public:
        virtual ~ICanvas() = default;

        virtual void SetColor(Color color);

        void MoveTo(double x, double y);
        virtual void LineTo(double x, double y) = 0;

        virtual void Fill() = 0;

        virtual void FillEllipse(double cx, double cy, double rx, double ry) = 0;

        virtual void DrawEllipse(double cx, double cy, double rx, double ry) = 0;
        virtual void DrawText(double left, double top, double fontSize, std::string text) = 0;

    protected:
        void TrackPath(double x, double y);

        [[nodiscard]] std::vector<Point> TakePath();
        [[nodiscard]] const std::vector<Point>& GetPath() const;

        Color m_color = Color("#FFFFFF");
        double m_x = 0.0;
        double m_y = 0.0;

    private:
        std::vector<Point> m_path;
    };
}