#include <sstream>
#include "../Catch/Catch.hpp"
#include "../Observer/Event.h"
#include "../Picture/Picture.h"

#include "../Shape/ShapeGeometry/ShapeGeometries/Circle/Circle.h"
#include "../Shape/ShapeGeometry/ShapeGeometries/Rectangle/Rectangle.h"

TEST_CASE("Обработчик события добавления фигуры вызывается при добавлении фигуры") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int addCount = 0;
    auto sub = picture.SubscribeToShapeAdded([&addCount](const shapes::IShape&) {
        addCount++;
    });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    
    REQUIRE(addCount == 0);
    picture.AddShape(circleShape);
    REQUIRE(addCount == 1);
}

TEST_CASE("Обработчик добавления фигуры не вызывается при перемещении фигуры") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int addCount = 0;
    int moveCount = 0;
    auto sub1 = picture.SubscribeToShapeAdded([&addCount](const shapes::IShape&) {
        addCount++;
    });
    auto sub2 = picture.SubscribeToShapeMoved([&moveCount](const shapes::IShape&, double, double) {
        moveCount++;
    });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    picture.AddShape(circleShape);

    REQUIRE(addCount == 1);
    REQUIRE(moveCount == 0);

    picture.MoveShape("1", 10, 10);
    REQUIRE(addCount == 1);
    REQUIRE(moveCount == 1);
}

TEST_CASE("Обработчик перемещения получает необходимые данные о перемещении") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    shapes::IShape* movedShape = nullptr;
    double dxReceived = 0, dyReceived = 0;
    auto sub = picture.SubscribeToShapeMoved([&](const shapes::IShape& shape, double dx, double dy) {
        movedShape = const_cast<shapes::IShape*>(&shape);
        dxReceived = dx;
        dyReceived = dy;
    });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    picture.AddShape(circleShape);

    picture.MoveShape("1", 15.5, -20.0);

    REQUIRE(movedShape != nullptr);
    REQUIRE(movedShape->GetId() == "1");
    REQUIRE(dxReceived == 15.5);
    REQUIRE(dyReceived == -20.0);
}

TEST_CASE("Один объект может одновременно реагировать на несколько различных событий через разные обработчики") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int addCount = 0, moveCount = 0, colorCount = 0;
    
    auto sub1 = picture.SubscribeToShapeAdded([&](const shapes::IShape&) { addCount++; });
    auto sub2 = picture.SubscribeToShapeMoved([&](const shapes::IShape&, double, double) { moveCount++; });
    auto sub3 = picture.SubscribeToShapeColorChanged([&](const shapes::IShape&, const gfx::Color&) { colorCount++; });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    picture.AddShape(circleShape);

    REQUIRE(addCount == 1);
    REQUIRE(moveCount == 0);
    REQUIRE(colorCount == 0);

    picture.MoveShape("1", 10, 10);
    REQUIRE(addCount == 1);
    REQUIRE(moveCount == 1);
    REQUIRE(colorCount == 0);

    picture.ChangeColor("1", gfx::Color("#FF0000"));
    REQUIRE(addCount == 1);
    REQUIRE(moveCount == 1);
    REQUIRE(colorCount == 1);
}

TEST_CASE("На один тип события можно подписать несколько обработчиков") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int count1 = 0, count2 = 0;
    auto sub1 = picture.SubscribeToShapeAdded([&](const shapes::IShape&) { count1++; });
    auto sub2 = picture.SubscribeToShapeAdded([&](const shapes::IShape&) { count2++; });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    picture.AddShape(circleShape);

    REQUIRE(count1 == 1);
    REQUIRE(count2 == 1);
}

TEST_CASE("Подписки разных типов событий отключаются независимо друг от друга") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int addCount = 0, moveCount = 0;
    auto sub1 = picture.SubscribeToShapeAdded([&](const shapes::IShape&) { addCount++; });
    auto sub2 = picture.SubscribeToShapeMoved([&](const shapes::IShape&, double, double) { moveCount++; });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    picture.AddShape(circleShape);

    REQUIRE(addCount == 1);
    REQUIRE(moveCount == 0);

    sub1.Disconnect();

    shapes::IShape rectangle("2", gfx::Color("#F0F0F0"), std::make_unique<shapes::Rectangle>(100, 100, 100, 100));
    picture.AddShape(rectangle);
    picture.MoveShape("1", 10, 10);

    REQUIRE(addCount == 1);
    REQUIRE(moveCount == 1);
}

TEST_CASE("RAII-объект подписки корректно работает для каждого типа события") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int addCount = 0, moveCount = 0, colorCount = 0, removeCount = 0;

    {
        auto sub1 = picture.SubscribeToShapeAdded([&](const shapes::IShape&) { addCount++; });
        auto sub2 = picture.SubscribeToShapeMoved([&](const shapes::IShape&, double, double) { moveCount++; });
        auto sub3 = picture.SubscribeToShapeColorChanged([&](const shapes::IShape&, const gfx::Color&) { colorCount++; });
        auto sub4 = picture.SubscribeToShapeRemoved([&](const shapes::IShape&) { removeCount++; });

        shapes::Circle circle(10, 100, 100);
        shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
        picture.AddShape(circleShape);

        REQUIRE(addCount == 1);
        REQUIRE(moveCount == 0);
        REQUIRE(colorCount == 0);
        REQUIRE(removeCount == 0);

        picture.MoveShape("1", 10, 10);
        REQUIRE(moveCount == 1);

        picture.ChangeColor("1", gfx::Color("#FF0000"));
        REQUIRE(colorCount == 1);

        picture.DeleteShape("1");
        REQUIRE(removeCount == 1);
    }

    shapes::IShape rectangle("2", gfx::Color("#F0F0F0"), std::make_unique<shapes::Rectangle>(100, 100, 100, 100));
    picture.AddShape(rectangle);
    REQUIRE(addCount == 1);
}

TEST_CASE("Отключение обработчика во время рассылки сохраняет семантику предыдущих заданий") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int count1 = 0, count2 = 0;
    
    std::function<void(const shapes::IShape&)> handler1;
    EventSubscription<const shapes::IShape&> sub1;
    
    handler1 = [&](const shapes::IShape&) {
        count1++;
        if (count1 == 1) {
            sub1.Disconnect();
        }
    };
    
    sub1 = picture.SubscribeToShapeAdded(handler1);
    auto sub2 = picture.SubscribeToShapeAdded([&](const shapes::IShape&) { count2++; });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    picture.AddShape(circleShape);

    REQUIRE(count1 == 1);
    REQUIRE(count2 == 1);

    shapes::IShape rectangle("2", gfx::Color("#F0F0F0"), std::make_unique<shapes::Rectangle>(100, 100, 100, 100));
    picture.AddShape(rectangle);

    REQUIRE(count1 == 1);
    REQUIRE(count2 == 2);
}

TEST_CASE("Добавление обработчика во время рассылки начинает действовать только со следующего события") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int count1 = 0, count2 = 0;
    std::vector<EventSubscription<const shapes::IShape&>> subscriptions;
    
    auto sub1 = picture.SubscribeToShapeAdded([&](const shapes::IShape&) {
        count1++;
        if (count1 == 1) {
            subscriptions.push_back(picture.SubscribeToShapeAdded([&](const shapes::IShape&) {
                count2++;
            }));
        }
    });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    picture.AddShape(circleShape);

    REQUIRE(count1 == 1);
    REQUIRE(count2 == 0);

    shapes::IShape rectangle("2", gfx::Color("#F0F0F0"), std::make_unique<shapes::Rectangle>(100, 100, 100, 100));
    picture.AddShape(rectangle);

    REQUIRE(count1 == 2);
    REQUIRE(count2 == 1);
}

TEST_CASE("Изменение Shape напрямую приводит к соответствующему событию Picture") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int moveCount = 0, colorCount = 0;
    auto sub1 = picture.SubscribeToShapeMoved([&](const shapes::IShape&, double, double) { moveCount++; });
    auto sub2 = picture.SubscribeToShapeColorChanged([&](const shapes::IShape&, const gfx::Color&) { colorCount++; });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    picture.AddShape(circleShape);

    REQUIRE(moveCount == 0);
    REQUIRE(colorCount == 0);

    auto& shape = picture.GetShape("1");
    shape.Move(10, 10);
    shape.SetColor(gfx::Color("#FF0000"));

    REQUIRE(moveCount == 1);
    REQUIRE(colorCount == 1);
}

TEST_CASE("Обработчик события удаления фигуры вызывается при удалении фигуры") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int removeCount = 0;
    shapes::IShape* removedShape = nullptr;
    auto sub = picture.SubscribeToShapeRemoved([&](const shapes::IShape& shape) {
        removeCount++;
        removedShape = const_cast<shapes::IShape*>(&shape);
    });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    picture.AddShape(circleShape);

    REQUIRE(removeCount == 0);
    picture.DeleteShape("1");
    REQUIRE(removeCount == 1);
    REQUIRE(removedShape != nullptr);
    REQUIRE(removedShape->GetId() == "1");
}

TEST_CASE("Обработчик события изменения цвета вызывается при изменении цвета") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int colorCount = 0;
    gfx::Color receivedColor{"#000000"};
    auto sub = picture.SubscribeToShapeColorChanged([&](const shapes::IShape&, const gfx::Color& color) {
        colorCount++;
        receivedColor = color;
    });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    picture.AddShape(circleShape);

    REQUIRE(colorCount == 0);
    picture.ChangeColor("1", gfx::Color("#FF0000"));
    REQUIRE(colorCount == 1);
    REQUIRE(receivedColor.GetColor() == "#FF0000");
}

TEST_CASE("Обработчик события изменения геометрии вызывается при изменении геометрии") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int geomCount = 0;
    auto sub = picture.SubscribeToShapeGeometryChanged([&](const shapes::IShape&, const shapes::IShapeGeometry&) {
        geomCount++;
    });

    shapes::Circle circle(10, 100, 100);
    shapes::IShape circleShape("1", gfx::Color("#F0F0F0"), std::make_unique<shapes::Circle>(circle));
    picture.AddShape(circleShape);

    REQUIRE(geomCount == 0);
    picture.ChangeShape("1", std::make_unique<shapes::Rectangle>(100, 100, 50, 50));
    REQUIRE(geomCount == 1);
}
