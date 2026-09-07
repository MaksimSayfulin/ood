#pragma once
#include <memory>
#include <string>
#include "../ShapeGeometry/IShapeGeometry.h"

namespace shapes {
    class IShape {
    public:
        IShape(std::string id, gfx::Color color, std::unique_ptr<IShapeGeometry> geometry);
        virtual ~IShape() = default;

        IShape(const IShape& other);
        IShape& operator=(const IShape& other);

        IShape(IShape&&) noexcept = default;
        IShape& operator=(IShape&&) noexcept = default;

        [[nodiscard]] IShape Clone() const;

        [[nodiscard]] std::string GetId() const;
        [[nodiscard]] gfx::Color GetColor() const;
        [[nodiscard]] const IShapeGeometry* GetGeometry() const;

        void SetColor(const gfx::Color &color);
        void SetGeometry(std::unique_ptr<IShapeGeometry> geometry);

        void Move(double x, double y) const;
        void Draw(gfx::ICanvas &canvas) const;

    private:
        std::string m_id;
        gfx::Color m_color;
        std::unique_ptr<IShapeGeometry> m_geometry;
    };
}