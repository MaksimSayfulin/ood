#include "IShape.h"

#include <format>
#include <random>
#include <utility>

shapes::IShape::IShape(std::string id, gfx::Color color, std::unique_ptr<IShapeGeometry> geometry)
    : m_id(std::move(id))
    , m_color(std::move(color))
    , m_geometry(std::move(geometry)) {}

shapes::IShape::IShape(const IShape& other)
    : m_id(other.m_id)
    , m_color(other.m_color)
    , m_geometry(other.m_geometry ? std::unique_ptr<IShapeGeometry>(other.m_geometry->Clone()) : nullptr) {}

shapes::IShape& shapes::IShape::operator=(const IShape& other) {
    if (this != &other) {
        m_id = other.m_id;
        m_color = other.m_color;
        m_geometry = other.m_geometry ? std::unique_ptr<IShapeGeometry>(other.m_geometry->Clone()) : nullptr;
    }
    return *this;
}

namespace {
    std::string GenerateRandomId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, 999999);

        return std::format("clone_{}", distr(gen));
    }
}

shapes::IShape shapes::IShape::Clone() const {
    return {
        GenerateRandomId(),
        m_color,
        m_geometry ? std::unique_ptr<IShapeGeometry>(m_geometry->Clone()) : nullptr
    };
}

std::string shapes::IShape::GetId() const {
    return m_id;
}

gfx::Color shapes::IShape::GetColor() const {
    return m_color;
}

const shapes::IShapeGeometry *shapes::IShape::GetGeometry() const {
    return m_geometry.get();
}

void shapes::IShape::SetColor(const gfx::Color &color) {
    m_color = color;
}

void shapes::IShape::SetGeometry(std::unique_ptr<IShapeGeometry> geometry) {
    m_geometry = std::move(geometry);
}

void shapes::IShape::Move(const double x, const double y) const {
    if (m_geometry) {
        m_geometry->Move(x, y);
    }
}

void shapes::IShape::Draw(gfx::ICanvas &canvas) const {
    if (m_geometry) {
        canvas.SetColor(m_color);
        m_geometry->Draw(canvas);
    }
}