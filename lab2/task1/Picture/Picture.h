#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../Graphics/Canvas/ICanvas/ICanvas.h"
#include "../Observer/Event.h"
#include "../Shape/IShape/IShape.h"
#include "../Shape/ShapeGeometry/IShapeGeometry.h"

namespace shapes {
    class Picture {
    public:
        explicit Picture(gfx::ICanvas &canvas);
        Picture(const Picture& other);

        ~Picture();

        void AddShape(const std::string& id, const gfx::Color& color, std::unique_ptr<IShapeGeometry> geometry);
        void AddShape(const IShape &shape);

        void MoveShape(const std::string& id, double dx, double dy);
        void MovePicture(double dx, double dy);
        void DeleteShape(const std::string& id);
        void List() const;

        void ChangeColor(const std::string &id, const gfx::Color &color);
        void ChangeShape(const std::string &id, std::unique_ptr<IShapeGeometry> geometry);

        void DrawShape(const std::string &id);
        void DrawPicture() const;

        [[nodiscard]] size_t GetShapesCount() const;
        [[nodiscard]] IShape& GetShape(const std::string& id);

        EventSubscription<const IShape&> SubscribeToShapeAdded(std::function<void(const IShape&)> handler);
        EventSubscription<const IShape&> SubscribeToShapeRemoved(std::function<void(const IShape&)> handler);
        EventSubscription<const IShape&, double, double> SubscribeToShapeMoved(std::function<void(const IShape&, double, double)> handler);
        EventSubscription<const IShape&, const gfx::Color&> SubscribeToShapeColorChanged(std::function<void(const IShape&, const gfx::Color&)> handler);
        EventSubscription<const IShape&, const IShapeGeometry&> SubscribeToShapeGeometryChanged(std::function<void(const IShape&, const IShapeGeometry&)> handler);

    private:
        gfx::ICanvas& m_canvas;
        std::vector<IShape> m_shapes;

        Event<const IShape&> m_shapeAddedEvent;
        Event<const IShape&> m_shapeRemovedEvent; // Вернуть только то, что нужно
        Event<const IShape&, double, double> m_shapeMovedEvent;
        Event<const IShape&, const gfx::Color&> m_shapeColorChangedEvent;
        Event<const IShape&, const IShapeGeometry&> m_shapeGeometryChangedEvent;

        std::vector<EventSubscription<const IShape&, double, double>> m_shapeMoveSubscriptions;
        std::vector<EventSubscription<const IShape&, const gfx::Color&>> m_shapeColorSubscriptions;
        std::vector<EventSubscription<const IShape&, const IShapeGeometry&>> m_shapeGeometrySubscriptions;

        IShape* FindShape(const std::string& id);
        void SubscribeToShapeEvents(IShape& shape);
        void UnsubscribeFromShapeEvents(IShape& shape);
    };
}
