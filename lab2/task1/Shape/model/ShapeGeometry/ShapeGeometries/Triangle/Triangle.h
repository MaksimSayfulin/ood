#pragma once
#include <array>
#include "../../IShapeGeometry.h"

namespace shapes {
    class Triangle : public IShapeGeometry {
    public:
        explicit Triangle(gfx::Rect bounds);

        void Draw(gfx::ICanvas& canvas) const override;
        void Move(double x, double y) override;
        [[nodiscard]] gfx::Rect GetBounds() const override;
        void SetBounds(const gfx::Rect& bounds) override;
        [[nodiscard]] bool Contains(double x, double y) const override;

        [[nodiscard]] std::string GetParameters() const override;
        [[nodiscard]] std::string GetShapeName() const override;
        [[nodiscard]] IShapeGeometry* Clone() const override;

    private:
        [[nodiscard]] std::array<gfx::Point, 3> GetPoints() const;

        gfx::Rect m_bounds;
    };
}