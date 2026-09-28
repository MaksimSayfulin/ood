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
    };
}