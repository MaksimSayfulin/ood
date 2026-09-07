#include "Picture.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

shapes::Picture::Picture(gfx::ICanvas& canvas) : m_canvas(canvas) {
}

void shapes::Picture::AddShape(const std::string& id, const gfx::Color& color, std::unique_ptr<IShapeGeometry> geometry) {
    if (FindShape(id) != nullptr) {
        throw std::invalid_argument("Shape already exists");
    }
    m_shapes.emplace_back(id, color, std::move(geometry));
}

void shapes::Picture::AddShape(const IShape& shape) {
    if (FindShape(shape.GetId()) != nullptr) {
        throw std::invalid_argument("Shape already exists");
    }
    m_shapes.push_back(shape);
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
    auto it = std::ranges::find_if(m_shapes,
                                   [&id](const IShape& shape) {
                                       return shape.GetId() == id;
                                   });

    if (it != m_shapes.end()) {
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

shapes::IShape*shapes::Picture::FindShape(const std::string& id) {
    for (auto& shape : m_shapes) {
        if (shape.GetId() == id) {
            return &shape;
        }
    }
    return nullptr;
}