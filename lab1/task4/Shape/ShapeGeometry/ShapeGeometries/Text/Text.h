#pragma once
#include <string>
#include "../../IShapeGeometry.h"

namespace shapes {
    class Text : public IShapeGeometry {
    public:
        Text(double &&left, double &&top, double &&scale, std::string &&text);
        void Draw(gfx::ICanvas &canvas) const override;
        void Move(double x, double y) override;

        [[nodiscard]] std::string GetParameters() const override;
        [[nodiscard]] std::string GetShapeName() const override;
        [[nodiscard]] IShapeGeometry * Clone() const override;


    private:
        double m_left = 0.0;
        double m_top = 0.0;
        double m_scale = 1.0;
        std::string m_text;
    };
}