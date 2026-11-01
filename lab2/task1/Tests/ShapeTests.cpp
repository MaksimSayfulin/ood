#define CATCH_CONFIG_MAIN
#include "../Catch/Catch.hpp"
#include "../Graphics/Canvas/ICanvas/ICanvas.h"
#include "../Shape/model/IShape/IShape.h"
#include "../Shape/model/ShapeGeometry/IShapeGeometry.h"

#include <memory>
#include <string>

class MockCanvas : public gfx::ICanvas {
public:
    gfx::Color lastColor{ "#000000" };
    bool setColorCalled = false;

    void SetColor(const gfx::Color color) override {
        lastColor = color;
        setColorCalled = true;
    }
    void LineTo(double, double) override {}
    void Fill() override {}
    void FillEllipse(double, double, double, double) override {}
    void DrawEllipse(double, double, double, double) override {}
    void DrawText(double, double, double, std::string) override {}
};

class MockGeometry : public shapes::IShapeGeometry {
public:
    mutable bool drawCalled = false;
    bool moveCalled = false;
    double accumulatedX = 0.0;
    double accumulatedY = 0.0;
    gfx::Rect m_bounds{};

    void Draw(gfx::ICanvas&) const override {
        drawCalled = true;
    }

    void Move(double x, double y) override {
        moveCalled = true;
        accumulatedX += x;
        accumulatedY += y;
    }

    [[nodiscard]] bool Contains(double, double) const override {
        return false;
    }

    [[nodiscard]] gfx::Rect GetBounds() const override {
        return m_bounds;
    }

    void SetBounds(const gfx::Rect& bounds) override {
        m_bounds = bounds;
    }

    [[nodiscard]] std::string GetParameters() const override {
        return "mock_param";
    }

    [[nodiscard]] std::string GetShapeName() const override {
        return "mock";
    }

    [[nodiscard]] IShapeGeometry* Clone() const override {
        return new MockGeometry(*this);
    }
};

TEST_CASE("IShape: Инициализация и геттеры", "[IShape]") {
    auto geomPtr = std::make_unique<MockGeometry>();
    const auto* rawGeomPtr = geomPtr.get();

    shapes::IShape shape("shape_1", gfx::Color("#FF0000"), std::move(geomPtr));

    SECTION("Инициализация корректно сохраняет ID, цвет и геометрию") {
        REQUIRE(shape.GetId() == "shape_1");
        REQUIRE(shape.GetColor().GetColor() == "#FF0000");
        REQUIRE(shape.GetGeometry() == rawGeomPtr);
    }
}

TEST_CASE("IShape: Изменение состояния (Сеттеры)", "[IShape]") {
    auto geomPtr1 = std::make_unique<MockGeometry>();
    shapes::IShape shape("shape_1", gfx::Color("#FF0000"), std::move(geomPtr1));

    SECTION("Изменение цвета фигуры") {
        shape.SetColor(gfx::Color("#00FF00"));
        REQUIRE(shape.GetColor().GetColor() == "#00FF00");
    }

    SECTION("Замена геометрии обновляет указатель и уничтожает старую") {
        auto geomPtr2 = std::make_unique<MockGeometry>();
        const auto* rawGeomPtr2 = geomPtr2.get();

        shape.SetGeometry(std::move(geomPtr2));
        REQUIRE(shape.GetGeometry() == rawGeomPtr2);
    }
}

TEST_CASE("IShape: Вызовы Move и Draw", "[IShape]") {
    auto geomPtr = std::make_unique<MockGeometry>();
    auto* mockGeom = geomPtr.get();
    shapes::IShape shape("shape_1", gfx::Color("#123456"), std::move(geomPtr));

    SECTION("Move передает смещение внутрь геометрии") {
        shape.Move(15.5, -10.0);
        REQUIRE(mockGeom->moveCalled);
        REQUIRE(mockGeom->accumulatedX == 15.5);
        REQUIRE(mockGeom->accumulatedY == -10.0);
    }

    SECTION("Draw устанавливает цвет на холсте и запрашивает отрисовку у геометрии") {
        MockCanvas canvas;
        shape.Draw(canvas);

        REQUIRE(canvas.setColorCalled);
        REQUIRE(canvas.lastColor.GetColor() == "#123456");
        REQUIRE(mockGeom->drawCalled);
    }
}

TEST_CASE("IShape: Семантика копирования (Глубокое копирование)", "[IShape]") {
    auto geomPtr = std::make_unique<MockGeometry>();
    shapes::IShape original("orig_id", gfx::Color("#AABBCC"), std::move(geomPtr));

    SECTION("Конструктор копирования сохраняет исходный ID и делает глубокую копию геометрии") {
        shapes::IShape copy(original);

        REQUIRE(copy.GetId() == original.GetId());
        REQUIRE(copy.GetColor().GetColor() == original.GetColor().GetColor());
        REQUIRE(copy.GetGeometry() != original.GetGeometry());
        REQUIRE(copy.GetGeometry() != nullptr);
    }

    SECTION("Оператор присваивания копированием корректно обновляет данные и геометрию") {
        auto otherGeom = std::make_unique<MockGeometry>();
        shapes::IShape target("target_id", gfx::Color("#000000"), std::move(otherGeom));

        target = original;

        REQUIRE(target.GetId() == "orig_id");
        REQUIRE(target.GetColor().GetColor() == "#AABBCC");
        REQUIRE(target.GetGeometry() != original.GetGeometry());
    }

    SECTION("Модификация копии не влияет на оригинальный объект") {
        shapes::IShape copy(original);
        copy.SetColor(gfx::Color("#FFFFFF"));
        copy.Move(100.0, 100.0);

        REQUIRE(original.GetColor().GetColor() == "#AABBCC");
        const auto* origGeom = dynamic_cast<const MockGeometry*>(original.GetGeometry());
        REQUIRE_FALSE(origGeom->moveCalled);
    }
}

TEST_CASE("IShape: Семантика перемещения (Move)", "[IShape]") {
    auto geomPtr = std::make_unique<MockGeometry>();
    const auto* rawGeom = geomPtr.get();
    shapes::IShape source("movable_id", gfx::Color("#112233"), std::move(geomPtr));

    SECTION("Конструктор перемещения передает владение ресурсами") {
        shapes::IShape destination(std::move(source));

        REQUIRE(destination.GetId() == "movable_id");
        REQUIRE(destination.GetColor().GetColor() == "#112233");
        REQUIRE(destination.GetGeometry() == rawGeom);
    }

    SECTION("Оператор присваивания перемещением передает владение ресурсами") {
        auto dummyGeom = std::make_unique<MockGeometry>();
        shapes::IShape destination("dummy_id", gfx::Color("#000000"), std::move(dummyGeom));

        destination = std::move(source);

        REQUIRE(destination.GetId() == "movable_id");
        REQUIRE(destination.GetColor().GetColor() == "#112233");
        REQUIRE(destination.GetGeometry() == rawGeom);
    }
}

TEST_CASE("IShape: Метод Clone()", "[IShape]") {
    auto geomPtr = std::make_unique<MockGeometry>();
    shapes::IShape original("original_id", gfx::Color("#FF00FF"), std::move(geomPtr));

    shapes::IShape cloned = original.Clone();

    SECTION("Clone создает объект с новым ID, но сохраняет цвет и дублирует геометрию") {
        REQUIRE(cloned.GetId() != original.GetId());
        REQUIRE(cloned.GetId().rfind("clone_", 0) == 0);
        REQUIRE(cloned.GetColor().GetColor() == "#FF00FF");
        REQUIRE(cloned.GetGeometry() != original.GetGeometry());
        REQUIRE(cloned.GetGeometry() != nullptr);
    }
}

TEST_CASE("IShape: Безопасность при работе с nullptr геометрией", "[IShape]") {
    shapes::IShape shapeWithNull("null_shape", gfx::Color("#000000"), nullptr);

    SECTION("Защита от падения при вызове Move и Draw на нулевой геометрии") {
        MockCanvas canvas;

        REQUIRE_NOTHROW(shapeWithNull.Move(10.0, 10.0));
        REQUIRE_NOTHROW(shapeWithNull.Draw(canvas));
        REQUIRE(shapeWithNull.GetGeometry() == nullptr);
    }

    SECTION("Клонирование фигуры с nullptr геометрией не вызывает аварийное завершение") {
        REQUIRE_NOTHROW([&]() {
            shapes::IShape clone = shapeWithNull.Clone();
            REQUIRE(clone.GetGeometry() == nullptr);
        }());
    }
}