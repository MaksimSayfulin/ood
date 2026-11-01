#pragma once
#include <string>
#include <boost/signals2/signal.hpp>

#include "../../../Graphics/Canvas/ICanvas/ICanvas.h"
#include "../../../Graphics/Geometry/Rect.h"

namespace shapes {
    class IShapeGeometry {
    public:
        virtual ~IShapeGeometry() = default;

        virtual void Draw(gfx::ICanvas& canvas) const = 0;
        virtual void Move(double x, double y) = 0;

        [[nodiscard]] virtual gfx::Rect GetBounds() const = 0;
        virtual void SetBounds(const gfx::Rect& bounds) = 0;

        [[nodiscard]] virtual bool Contains(double x, double y) const = 0;

        [[nodiscard]] virtual IShapeGeometry* Clone() const = 0;
        [[nodiscard]] virtual std::string GetParameters() const = 0;
        [[nodiscard]] virtual std::string GetShapeName() const = 0;
    };
}