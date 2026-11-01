#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "../../Graphics/Color/Color.h"
#include "../../Graphics/Geometry/Rect.h"
#include "IShape/IShape.h"

namespace shapes {
    class ShapeFactory {
    public:
        [[nodiscard]] static std::unique_ptr<IShapeGeometry> CreateGeometry(std::string_view type,
                                                                            const gfx::Rect& bounds);

        [[nodiscard]] static IShape CreateShape(std::string_view type,
                                                const gfx::Rect& bounds,
                                                const gfx::Color& color);

        [[nodiscard]] static bool IsSupported(std::string_view type);

        [[nodiscard]] static std::string GenerateShapeId();
    };
}