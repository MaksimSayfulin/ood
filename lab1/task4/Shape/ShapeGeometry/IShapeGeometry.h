#pragma once

#include "../../Graphics/Canvas/CanvasSVG/CanvasSvg.h"

namespace shapes {
    class IShapeGeometry {
    public:
        virtual ~IShapeGeometry(); //lack of memory
        virtual void Draw(gfx::ICanvas& canvas) const = 0;
        virtual void Move(double x, double y) = 0;
        virtual IShapeGeometry* CreateFrom(IShapeGeometry* shape) = 0;
        [[nodiscard]] virtual IShapeGeometry* Clone() const = 0;
        [[nodiscard]] virtual std::string GetParameters() const = 0;
        [[nodiscard]] virtual std::string GetShapeName() const = 0;

    protected:
        double m_x1 = 0.0;
        double m_y1 = 0.0;
        double m_x2 = 0.0;
        double m_y2 = 0.0;
        double m_x3 = 0.0;
        double m_y3 = 0.0;

        double m_left = 0.0;
        double m_top = 0.0;
        double m_scale = 1.0;
        std::string m_text;

        double m_height = 0.0;
        double m_width = 0.0;

        double m_radius = 0.0;
        double m_x = 0.0;
        double m_y = 0.0;
    };
}