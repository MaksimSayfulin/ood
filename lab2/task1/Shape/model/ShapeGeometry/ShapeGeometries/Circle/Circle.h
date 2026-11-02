#pragma once
#include "../../IShapeGeometry.h"

namespace shapes {
    class Circle : public IShapeGeometry {
    public:
        explicit Circle(double radius, double x, double y);
        void Draw(gfx::ICanvas &canvas) const override;
        void Move(double x, double y) override;

        [[nodiscard]] std::string GetParameters() const override;
        [[nodiscard]] std::string GetShapeName() const override;
        [[nodiscard]] IShapeGeometry* Clone() const override;

    private:
        double m_radius = 0;
        double m_x = 0;
        double m_y = 0;
    };
}