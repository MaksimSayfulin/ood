#include "Picture.h"

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <utility>

shapes::Picture::Picture(gfx::ICanvas& canvas) : m_canvas(canvas) {
}

shapes::Picture::Picture(const Picture& other)
    : m_canvas(other.m_canvas)
      , m_shapes(other.m_shapes)
{
    m_shapeSubscriptions.reserve(m_shapes.size());
    for (auto& shape : m_shapes) {
        m_shapeSubscriptions.push_back(SubscribeToShapeEvents(shape));
    }
}

void shapes::Picture::AddShape(const std::string& id, const gfx::Color& color, std::unique_ptr<IShapeGeometry> geometry) {
    if (FindShape(id) != nullptr) {
        throw std::invalid_argument("Shape already exists");
    }
    m_shapes.emplace_back(id, color, std::move(geometry));
    m_shapeSubscriptions.push_back(SubscribeToShapeEvents(m_shapes.back()));
    m_shapeAddedEvent(m_shapes.back());
}

void shapes::Picture::AddShape(const IShape& shape) {
    if (FindShape(shape.GetId()) != nullptr) {
        throw std::invalid_argument("Shape already exists");
    }
    m_shapes.push_back(shape);
    m_shapeSubscriptions.push_back(SubscribeToShapeEvents(m_shapes.back()));
    m_shapeAddedEvent(m_shapes.back());
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
    }
}

void shapes::Picture::DeleteShape(const std::string& id) {
    const auto index = FindShapeIndex(id);
    if (index == m_shapes.size()) {
        throw std::invalid_argument("No such shape");
    }

    m_shapeRemovedEvent(m_shapes[index]);

    const auto position = static_cast<std::ptrdiff_t>(index);
    m_shapeSubscriptions.erase(m_shapeSubscriptions.begin() + position);
    m_shapes.erase(m_shapes.begin() + position);
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

boost::signals2::signal<void(const shapes::IShape &)> &shapes::Picture::OnShapeAdded() {
    return m_shapeAddedEvent;
}

boost::signals2::signal<void(const shapes::IShape &)> &shapes::Picture::OnShapeRemoved() {
    return m_shapeRemovedEvent;
}

boost::signals2::signal<void(const shapes::IShape &, double, double)> &shapes::Picture::OnShapeMoved() {
    return m_shapeMovedEvent;
}

boost::signals2::signal<void(const shapes::IShape &, const gfx::Color &)> &shapes::Picture::OnShapeColorChanged() {
    return m_shapeColorChangedEvent;
}

boost::signals2::signal<void(const shapes::IShape &, const shapes::IShapeGeometry &)> &shapes::Picture::OnShapeGeometryChanged() {
    return m_shapeGeometryChangedEvent;
}

shapes::IShape *shapes::Picture::FindShape(const std::string& id) {
    const auto index = FindShapeIndex(id);
    return index == m_shapes.size() ? nullptr : &m_shapes[index];
}

size_t shapes::Picture::FindShapeIndex(const std::string& id) const {
    for (size_t i = 0; i < m_shapes.size(); ++i) {
        if (m_shapes[i].GetId() == id) {
            return i;
        }
    }
    return m_shapes.size();
}

shapes::Picture::ShapeSubscriptions shapes::Picture::SubscribeToShapeEvents(IShape& shape) {
    ShapeSubscriptions subscriptions;

    subscriptions.moved = shape.OnMove().connect([this](const IShape& s, double dx, double dy) {
        m_shapeMovedEvent(s, dx, dy);
    });

    subscriptions.colorChanged = shape.OnColorChanged().connect([this](const IShape& s, const gfx::Color& color) {
        m_shapeColorChangedEvent(s, color);
    });

    subscriptions.geometryChanged = shape.OnGeometryChanged().connect([this](const IShape& s, const IShapeGeometry& geom) {
        m_shapeGeometryChangedEvent(s, geom);
    });

    return subscriptions;
}
