#include "Picture.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

shapes::Picture::Picture(gfx::ICanvas& canvas) : m_canvas(canvas) {
}

shapes::Picture::Picture(const Picture& other)
    : m_canvas(other.m_canvas)
      , m_shapes(other.m_shapes)
{
    for (auto& shape : m_shapes) {
        SubscribeToShapeEvents(shape);
    }
}

shapes::Picture::~Picture() {
    for (auto& shape : m_shapes) {
        UnsubscribeFromShapeEvents(shape);
    }
}

void shapes::Picture::AddShape(const std::string& id, const gfx::Color& color, std::unique_ptr<IShapeGeometry> geometry) {
    if (FindShape(id) != nullptr) {
        throw std::invalid_argument("Shape already exists");
    }
    m_shapes.emplace_back(id, color, std::move(geometry));
    SubscribeToShapeEvents(m_shapes.back());
    m_shapeAddedEvent.Notify(m_shapes.back());
}

void shapes::Picture::AddShape(const IShape& shape) {
    if (FindShape(shape.GetId()) != nullptr) {
        throw std::invalid_argument("Shape already exists");
    }
    m_shapes.push_back(shape);
    SubscribeToShapeEvents(m_shapes.back());
    m_shapeAddedEvent.Notify(m_shapes.back());
}

void shapes::Picture::MoveShape(const std::string& id, const double dx, const double dy) {
    const auto shape = FindShape(id);
    if (!shape) {
        throw std::invalid_argument("No such shape");
    }

    shape->Move(dx, dy);
}

void shapes::Picture::MovePicture(const double dx, const double dy) {
    for (auto& shape : m_shapes) {
        shape.Move(dx, dy);
        m_shapeMovedEvent.Notify(shape, dx, dy);
    }
}

void shapes::Picture::DeleteShape(const std::string& id) {
    const auto it = std::ranges::find_if(m_shapes,
                                         [&id](const IShape& shape) {
                                             return shape.GetId() == id;
                                         });

    if (it != m_shapes.end()) {
        UnsubscribeFromShapeEvents(*it);
        m_shapeRemovedEvent.Notify(*it);
        m_shapes.erase(it);
        return;
    }
    throw std::invalid_argument("No such shape");
}

void shapes::Picture::List() const {
    for (size_t i = 0; i < m_shapes.size(); i++) {
        const auto geometry = m_shapes[i].GetGeometry();
        if (!geometry) continue;

        std::cout << i << " " << geometry->GetShapeName() << " "
                << m_shapes[i].GetId() << " "
                << m_shapes[i].GetColor().GetColor() << " "
                << geometry->GetParameters() << std::endl;
    }
}

void shapes::Picture::ChangeColor(const std::string &id, const gfx::Color &color) {
    const auto shape = FindShape(id);
    if (!shape) {
        throw std::invalid_argument("No such shape");
    }

    shape->SetColor(color);
}

void shapes::Picture::ChangeShape(const std::string &id, std::unique_ptr<IShapeGeometry> geometry) {
    const auto shape = FindShape(id);
    if (!shape) {
        throw std::invalid_argument("No such shape");
    }

    shape->SetGeometry(std::move(geometry));
}

void shapes::Picture::DrawShape(const std::string &id) {
    const auto shape = FindShape(id);
    if (!shape) {
        throw std::invalid_argument("No such shape");
    }

    shape->Draw(m_canvas);
}

void shapes::Picture::DrawPicture() const {
    for (const auto& shape : m_shapes) {
        shape.Draw(m_canvas);
    }
}

size_t shapes::Picture::GetShapesCount() const {
    return m_shapes.size();
}

shapes::IShape& shapes::Picture::GetShape(const std::string& id) {
    const auto shape = FindShape(id);
    if (!shape) {
        throw std::invalid_argument("No such shape");
    }
    return *shape;
}

EventSubscription<const shapes::IShape&> shapes::Picture::SubscribeToShapeAdded(std::function<void(const IShape&)> handler) {
    return m_shapeAddedEvent.Subscribe(std::move(handler));
}

EventSubscription<const shapes::IShape&> shapes::Picture::SubscribeToShapeRemoved(std::function<void(const IShape&)> handler) {
    return m_shapeRemovedEvent.Subscribe(std::move(handler));
}

EventSubscription<const shapes::IShape&, double, double> shapes::Picture::SubscribeToShapeMoved(std::function<void(const IShape&, double, double)> handler) {
    return m_shapeMovedEvent.Subscribe(std::move(handler));
}

EventSubscription<const shapes::IShape&, const gfx::Color&> shapes::Picture::SubscribeToShapeColorChanged(std::function<void(const IShape&, const gfx::Color&)> handler) {
    return m_shapeColorChangedEvent.Subscribe(std::move(handler));
}

EventSubscription<const shapes::IShape&, const shapes::IShapeGeometry&> shapes::Picture::SubscribeToShapeGeometryChanged(std::function<void(const IShape&, const IShapeGeometry&)> handler) {
    return m_shapeGeometryChangedEvent.Subscribe(std::move(handler));
}

shapes::IShape*shapes::Picture::FindShape(const std::string& id) {
    for (auto& shape : m_shapes) {
        if (shape.GetId() == id) {
            return &shape;
        }
    }
    return nullptr;
}

void shapes::Picture::SubscribeToShapeEvents(IShape& shape) {
    m_shapeMoveSubscriptions.push_back(
        shape.OnMove().Subscribe([this](const IShape& s, double dx, double dy) {
            m_shapeMovedEvent.Notify(s, dx, dy);
        })
    );

    m_shapeColorSubscriptions.push_back(
        shape.OnColorChanged().Subscribe([this](const IShape& s, const gfx::Color& color) {
            m_shapeColorChangedEvent.Notify(s, color);
        })
    );

    m_shapeGeometrySubscriptions.push_back(
        shape.OnGeometryChanged().Subscribe([this](const IShape& s, const IShapeGeometry& geom) {
            m_shapeGeometryChangedEvent.Notify(s, geom);
        })
    );
}

void shapes::Picture::UnsubscribeFromShapeEvents(IShape& shape) {
    for (auto& sub : m_shapeMoveSubscriptions) {
        sub.Disconnect();
    }
    for (auto& sub : m_shapeColorSubscriptions) {
        sub.Disconnect();
    }
    for (auto& sub : m_shapeGeometrySubscriptions) {
        sub.Disconnect();
    }

    m_shapeMoveSubscriptions.clear();
    m_shapeColorSubscriptions.clear();
    m_shapeGeometrySubscriptions.clear();

    for (auto& s : m_shapes) {
        SubscribeToShapeEvents(s);
    }
}
