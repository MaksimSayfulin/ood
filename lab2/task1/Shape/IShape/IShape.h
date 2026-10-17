#pragma once
#include <memory>
#include <string>
#include <boost/signals2/signal.hpp>
#include "../ShapeGeometry/IShapeGeometry.h"

namespace shapes {
    class IShape {
    public:
        IShape(std::string id, gfx::Color color, std::unique_ptr<IShapeGeometry> geometry);
        ~IShape() = default;

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

        void Move(double x, double y);
        void Draw(gfx::ICanvas &canvas) const;

        boost::signals2::signal<void(const IShape&, double, double)>& OnMove();
        boost::signals2::signal<void(const IShape&, const gfx::Color&)>& OnColorChanged();
        boost::signals2::signal<void(const IShape&, const IShapeGeometry&)>& OnGeometryChanged();

    private:
        std::string m_id;
        gfx::Color m_color;
        std::unique_ptr<IShapeGeometry> m_geometry;

        boost::signals2::signal<void(const IShape&, double, double)> m_moveEvent;
        boost::signals2::signal<void(const IShape&, const gfx::Color&)> m_colorChangedEvent;
        boost::signals2::signal<void(const IShape&, const IShapeGeometry&)> m_geometryChangedEvent;
    };
}
