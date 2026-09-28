#pragma once

#include "../../Graphics/Canvas/CanvasSVG/CanvasSvg.h"

namespace shapes {
    class IShapeGeometry {
    public:
        virtual ~IShapeGeometry() = default;
        virtual void Draw(gfx::ICanvas& canvas) const = 0;
        virtual void Move(double x, double y) = 0;
        [[nodiscard]] virtual IShapeGeometry* Clone() const = 0;
        [[nodiscard]] virtual std::string GetParameters() const = 0;
        [[nodiscard]] virtual std::string GetShapeName() const = 0;
    };
}