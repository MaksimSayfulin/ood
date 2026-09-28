#pragma once
#include "../../IShapeGeometry.h"

namespace shapes {
    class Rectangle : public IShapeGeometry {
    public:
        explicit Rectangle(double &&top, double &&left, double &&width, double &&height);
        void Draw(gfx::ICanvas &canvas) const override;
        void Move(double x, double y) override;

        [[nodiscard]] std::string GetParameters() const override;
        [[nodiscard]] std::string GetShapeName() const override;
        [[nodiscard]] IShapeGeometry *Clone() const override;

    private:
        double m_top;
        double m_left;
        double m_width;
        double m_height;
    };
}