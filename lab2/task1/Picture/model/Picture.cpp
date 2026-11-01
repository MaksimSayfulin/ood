#include "Picture.h"

#include <cstddef>
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

void shapes::Picture::SetShapeBounds(const std::string& id, const gfx::Rect& bounds) {
    const auto shape = FindShape(id);
    if (!shape) {
        throw std::invalid_argument("No such shape");
    }

    shape->SetBounds(bounds);
}

void shapes::Picture::Clear() {
    while (!m_shapes.empty()) {
        DeleteShape(m_shapes.back().GetId());
    }
}

void shapes::Picture::ReplaceWith(const std::vector<IShape>& shapes) {
    Clear();

    for (const auto& shape : shapes) {
        AddShape(shape);
    }
}

void shapes::Picture::MoveShapeToIndex(const std::string& id, const size_t index) {
    const auto current = FindShapeIndex(id);
    if (current == m_shapes.size() || m_shapes.size() < 2) {
        return;
    }

    const size_t target = std::min(index, m_shapes.size() - 1);
    if (current == target) {
        return;
    }

    IShape shape = std::move(m_shapes[current]);
    m_shapes.erase(m_shapes.begin() + static_cast<std::ptrdiff_t>(current));
    m_shapes.insert(m_shapes.begin() + static_cast<std::ptrdiff_t>(target), std::move(shape));

    m_shapeSubscriptions.erase(m_shapeSubscriptions.begin() + static_cast<std::ptrdiff_t>(current));
    m_shapeSubscriptions.insert(m_shapeSubscriptions.begin() + static_cast<std::ptrdiff_t>(target),
                                ShapeSubscriptions{});
    m_shapeSubscriptions[target] = SubscribeToShapeEvents(m_shapes[target]);

    m_orderChangedEvent();
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

void shapes::Picture::DrawPicture(gfx::ICanvas& canvas) const {
    for (const auto& shape : m_shapes) {
        shape.Draw(canvas);
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

shapes::IShape *shapes::Picture::FindShapeAt(const double x, const double y) {
    for (auto it = m_shapes.rbegin(); it != m_shapes.rend(); ++it) {
        const auto* geometry = it->GetGeometry();
        if (geometry && geometry->Contains(x, y)) {
            return &*it;
        }
    }
    return nullptr;
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

boost::signals2::signal<void()> &shapes::Picture::OnOrderChanged() {
    return m_orderChangedEvent;
}

size_t shapes::Picture::GetShapeIndex(const std::string& id) const {
    return FindShapeIndex(id);
}

shapes::IShape *shapes::Picture::FindShape(const std::string& id) {
    const auto index = FindShapeIndex(id);
    return index == m_shapes.size() ? nullptr : &m_shapes[index];
}

const shapes::IShape *shapes::Picture::FindShape(const std::string& id) const {
    const auto index = FindShapeIndex(id);
    return index == m_shapes.size() ? nullptr : &m_shapes[index];
}

gfx::Rect shapes::Picture::GetShapeBounds(const std::string& id) const {
    const auto shape = FindShape(id);
    if (!shape) {
        throw std::invalid_argument("No such shape");
    }

    return shape->GetBounds();
}

const std::vector<shapes::IShape> &shapes::Picture::GetShapes() const {
    return m_shapes;
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
