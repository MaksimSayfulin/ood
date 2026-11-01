#include "ShapeFactory.h"

#include <format>
#include <random>
#include <stdexcept>

#include "ShapeGeometry/ShapeGeometries/Ellipse/Ellipse.h"
#include "ShapeGeometry/ShapeGeometries/Line/Line.h"
#include "ShapeGeometry/ShapeGeometries/Rectangle/Rectangle.h"
#include "ShapeGeometry/ShapeGeometries/Triangle/Triangle.h"

std::unique_ptr<shapes::IShapeGeometry> shapes::ShapeFactory::CreateGeometry(
    const std::string_view type, const gfx::Rect& bounds) {
    if (type == "rectangle") {
        return std::make_unique<Rectangle>(bounds);
    }
    if (type == "ellipse") {
        return std::make_unique<Ellipse>(bounds);
    }
    if (type == "triangle") {
        return std::make_unique<Triangle>(bounds);
    }
    if (type == "line") {
        return std::make_unique<Line>(bounds);
    }

    throw std::invalid_argument(std::format("Unknown shape type '{}'", type));
}

shapes::IShape shapes::ShapeFactory::CreateShape(const std::string_view type, const gfx::Rect& bounds, const gfx::Color& color) {
    return {GenerateShapeId(), color, CreateGeometry(type, bounds)};
}

bool shapes::ShapeFactory::IsSupported(const std::string_view type) {
    return type == "rectangle" || type == "ellipse" || type == "triangle" || type == "line";
}

std::string shapes::ShapeFactory::GenerateShapeId() {
    static std::random_device device;
    static std::mt19937 generator(device());
    static std::uniform_int_distribution distribution(0, 999999);

    return std::format("shape-{}", distribution(generator));
}