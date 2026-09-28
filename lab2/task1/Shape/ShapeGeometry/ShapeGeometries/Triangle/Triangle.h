#pragma once
#include "../../IShapeGeometry.h"

namespace shapes {
    class Triangle: public IShapeGeometry {
    public:
        Triangle(double &&x1, double &&y1, double &&x2, double &&y2, double &&x3, double &&y3);
        void Draw(gfx::ICanvas &canvas) const override;
        void Move(double x, double y) override;

        [[nodiscard]] std::string GetParameters() const override;
        [[nodiscard]] std::string GetShapeName() const override;
        [[nodiscard]] IShapeGeometry * Clone() const override;

    private:
        double m_x1;
        double m_y1;
        double m_x2;
        double m_y2;
        double m_x3;
        double m_y3;
    };
}