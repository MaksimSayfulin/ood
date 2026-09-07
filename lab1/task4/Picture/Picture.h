#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../Graphics/Canvas/ICanvas/ICanvas.h"
#include "../Shape/IShape/IShape.h"
#include "../Shape/ShapeGeometry/IShapeGeometry.h"

namespace shapes {
    class Picture {
    public:
        explicit Picture(gfx::ICanvas &canvas);

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

    private:
        gfx::ICanvas& m_canvas;
        std::vector<IShape> m_shapes;

        IShape* FindShape(const std::string& id);
    };
}