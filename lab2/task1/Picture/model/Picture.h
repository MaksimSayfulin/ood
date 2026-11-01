#pragma once
#include <memory>
#include <string>
#include <vector>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>

#include "../../Graphics/Canvas/ICanvas/ICanvas.h"
#include "../../Shape/model/IShape/IShape.h"
#include "../../Shape/model/ShapeGeometry/IShapeGeometry.h"

namespace shapes {
    class Picture {
    public:
        explicit Picture(gfx::ICanvas &canvas);
        Picture(const Picture& other);

        ~Picture() = default;

        void AddShape(const std::string& id, const gfx::Color& color, std::unique_ptr<IShapeGeometry> geometry);
        void AddShape(const IShape &shape);

        void MoveShape(const std::string& id, double dx, double dy);
        void MovePicture(double dx, double dy);
        void DeleteShape(const std::string& id);

        void MoveShapeToIndex(const std::string& id, size_t index);

        void ChangeColor(const std::string &id, const gfx::Color &color);
        void ChangeShape(const std::string &id, std::unique_ptr<IShapeGeometry> geometry);

        void SetShapeBounds(const std::string& id, const gfx::Rect& bounds);

        void Clear();
        void ReplaceWith(const std::vector<IShape>& shapes);

        void DrawPicture(gfx::ICanvas& canvas) const;

        [[nodiscard]] size_t GetShapesCount() const;
        [[nodiscard]] IShape& GetShape(const std::string& id);
        [[nodiscard]] gfx::Rect GetShapeBounds(const std::string& id) const;
        [[nodiscard]] const std::vector<IShape>& GetShapes() const;
        [[nodiscard]] size_t GetShapeIndex(const std::string& id) const;

        [[nodiscard]] IShape* FindShapeAt(double x, double y);

        boost::signals2::signal<void(const IShape&)>& OnShapeAdded();
        boost::signals2::signal<void(const IShape&)>& OnShapeRemoved();
        boost::signals2::signal<void(const IShape&, double, double)>& OnShapeMoved();
        boost::signals2::signal<void(const IShape&, const gfx::Color&)>& OnShapeColorChanged();
        boost::signals2::signal<void(const IShape&, const IShapeGeometry&)>& OnShapeGeometryChanged();
        boost::signals2::signal<void()>& OnOrderChanged();

    private:
        struct ShapeSubscriptions {
            boost::signals2::scoped_connection moved;
            boost::signals2::scoped_connection colorChanged;
            boost::signals2::scoped_connection geometryChanged;
        };

        gfx::ICanvas& m_canvas;
        std::vector<IShape> m_shapes;

        boost::signals2::signal<void(const IShape&)> m_shapeAddedEvent;
        boost::signals2::signal<void(const IShape&)> m_shapeRemovedEvent;
        boost::signals2::signal<void(const IShape&, double, double)> m_shapeMovedEvent;
        boost::signals2::signal<void(const IShape&, const gfx::Color&)> m_shapeColorChangedEvent;
        boost::signals2::signal<void(const IShape&, const IShapeGeometry&)> m_shapeGeometryChangedEvent;
        boost::signals2::signal<void()> m_orderChangedEvent;

        std::vector<ShapeSubscriptions> m_shapeSubscriptions;

        IShape* FindShape(const std::string& id);
        [[nodiscard]] const IShape* FindShape(const std::string& id) const;
        [[nodiscard]] size_t FindShapeIndex(const std::string& id) const;
        [[nodiscard]] ShapeSubscriptions SubscribeToShapeEvents(IShape& shape);
    };
}