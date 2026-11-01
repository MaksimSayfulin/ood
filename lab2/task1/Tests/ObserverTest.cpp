#include <sstream>
#include <string>
#include <vector>
#include <boost/signals2/connection.hpp>
#include "../Catch/Catch.hpp"
#include "../Graphics/Canvas/CanvasSVG/CanvasSvg.h"
#include "../Picture/model/Picture.h"

#include "../Shape/model/ShapeGeometry/ShapeGeometries/Ellipse/Ellipse.h"
#include "../Shape/model/ShapeGeometry/ShapeGeometries/Rectangle/Rectangle.h"

namespace {
    struct CallLog {
        std::string order;

        void Add(const std::string& entry) {
            order += entry;
        }
    };

    shapes::IShape MakeCircleShape(const std::string& id = "1") {
        return {id, gfx::Color("#F0F0F0"), std::make_unique<shapes::Ellipse>(gfx::Rect{10, 100, 200, 200})};
    }

    shapes::IShape MakeRectangleShape(const std::string& id = "2") {
        return {id, gfx::Color("#F0F0F0"), std::make_unique<shapes::Rectangle>(gfx::Rect{100, 100, 100, 100})};
    }
}

TEST_CASE("Обработчик события добавления фигуры вызывается при добавлении фигуры") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int addCount = 0;
    boost::signals2::scoped_connection sub = picture.OnShapeAdded().connect([&addCount](const shapes::IShape&) {
        addCount++;
    });

    picture.AddShape(MakeCircleShape());

    REQUIRE(addCount == 1);
}

TEST_CASE("Обработчик добавления фигуры не вызывается при перемещении фигуры") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int addCount = 0;
    int moveCount = 0;
    boost::signals2::scoped_connection sub1 = picture.OnShapeAdded().connect([&addCount](const shapes::IShape&) {
        addCount++;
    });
    boost::signals2::scoped_connection sub2 = picture.OnShapeMoved().connect([&moveCount](const shapes::IShape&, double, double) {
        moveCount++;
    });

    picture.AddShape(MakeCircleShape());

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
    boost::signals2::scoped_connection sub = picture.OnShapeMoved().connect([&](const shapes::IShape& shape, double dx, double dy) {
        movedShape = const_cast<shapes::IShape*>(&shape);
        dxReceived = dx;
        dyReceived = dy;
    });

    picture.AddShape(MakeCircleShape());
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

    boost::signals2::scoped_connection sub1 = picture.OnShapeAdded().connect([&](const shapes::IShape&) { addCount++; });
    boost::signals2::scoped_connection sub2 = picture.OnShapeMoved().connect([&](const shapes::IShape&, double, double) { moveCount++; });
    boost::signals2::scoped_connection sub3 = picture.OnShapeColorChanged().connect([&](const shapes::IShape&, const gfx::Color&) { colorCount++; });

    picture.AddShape(MakeCircleShape());

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
    boost::signals2::scoped_connection sub1 = picture.OnShapeAdded().connect([&](const shapes::IShape&) { count1++; });
    boost::signals2::scoped_connection sub2 = picture.OnShapeAdded().connect([&](const shapes::IShape&) { count2++; });

    picture.AddShape(MakeCircleShape());

    REQUIRE(count1 == 1);
    REQUIRE(count2 == 1);
}

TEST_CASE("Обработчики одного сигнала вызываются в порядке подключения") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    CallLog log;
    picture.OnShapeAdded().connect([&](const shapes::IShape&) { log.Add("a"); });
    picture.OnShapeAdded().connect([&](const shapes::IShape&) { log.Add("b"); });
    picture.OnShapeAdded().connect([&](const shapes::IShape&) { log.Add("c"); });

    picture.AddShape(MakeCircleShape());

    REQUIRE(log.order == "abc");
}

TEST_CASE("Подписки разных типов событий отключаются независимо друг от друга") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int addCount = 0, moveCount = 0;
    boost::signals2::scoped_connection sub1 = picture.OnShapeAdded().connect([&](const shapes::IShape&) { addCount++; });
    boost::signals2::scoped_connection sub2 = picture.OnShapeMoved().connect([&](const shapes::IShape&, double, double) { moveCount++; });

    picture.AddShape(MakeCircleShape());

    REQUIRE(addCount == 1);
    REQUIRE(moveCount == 0);

    sub1.disconnect();

    picture.AddShape(MakeRectangleShape());
    picture.MoveShape("1", 10, 10);

    REQUIRE(addCount == 1);
    REQUIRE(moveCount == 1);
}

TEST_CASE("Отключённый обработчик больше не вызывается") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int count = 0;
    boost::signals2::scoped_connection sub = picture.OnShapeAdded().connect([&](const shapes::IShape&) { count++; });

    picture.AddShape(MakeCircleShape());
    REQUIRE(count == 1);

    sub.disconnect();
    REQUIRE_FALSE(sub.connected());

    picture.AddShape(MakeRectangleShape());
    REQUIRE(count == 1);
}

TEST_CASE("scoped_connection отключает обработчик при выходе из области видимости") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int count = 0;
    {
        boost::signals2::scoped_connection sub =
            picture.OnShapeAdded().connect([&](const shapes::IShape&) { count++; });

        picture.AddShape(MakeCircleShape());
        REQUIRE(count == 1);
        REQUIRE(sub.connected());
    }

    picture.AddShape(MakeRectangleShape());
    REQUIRE(count == 1);
}

TEST_CASE("Уничтожение объекта, управляющего подпиской, не оставляет некорректного соединения") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int count = 0;
    {
        struct Listener {
            explicit Listener(shapes::Picture& picture, int& counter)
                : m_sub(picture.OnShapeAdded().connect([&counter](const shapes::IShape&) { counter++; })) {}

            boost::signals2::scoped_connection m_sub;
        } listener(picture, count);

        picture.AddShape(MakeCircleShape());
        REQUIRE(count == 1);
    }

    picture.AddShape(MakeRectangleShape());
    REQUIRE(count == 1);
}

TEST_CASE("RAII-объект подписки корректно работает для каждого типа события") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int addCount = 0, moveCount = 0, colorCount = 0, removeCount = 0;

    {
        boost::signals2::scoped_connection sub1 = picture.OnShapeAdded().connect([&](const shapes::IShape&) { addCount++; });
        boost::signals2::scoped_connection sub2 = picture.OnShapeMoved().connect([&](const shapes::IShape&, double, double) { moveCount++; });
        boost::signals2::scoped_connection sub3 = picture.OnShapeColorChanged().connect([&](const shapes::IShape&, const gfx::Color&) { colorCount++; });
        boost::signals2::scoped_connection sub4 = picture.OnShapeRemoved().connect([&](const shapes::IShape&) { removeCount++; });

        picture.AddShape(MakeCircleShape());

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

    picture.AddShape(MakeRectangleShape());
    REQUIRE(addCount == 1);
}

TEST_CASE("Обработчик может отключить собственную подписку во время рассылки") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int count1 = 0, count2 = 0;

    boost::signals2::scoped_connection sub1;
    sub1 = picture.OnShapeAdded().connect([&](const shapes::IShape&) {
        count1++;
        if (count1 == 1) {
            sub1.disconnect();
        }
    });
    boost::signals2::scoped_connection sub2 = picture.OnShapeAdded().connect([&](const shapes::IShape&) { count2++; });

    picture.AddShape(MakeCircleShape());

    REQUIRE(count1 == 1);
    REQUIRE(count2 == 1);

    picture.AddShape(MakeRectangleShape());

    REQUIRE(count1 == 1);
    REQUIRE(count2 == 2);
}

TEST_CASE("Обработчик может отключить другой, ещё не вызванный обработчик во время рассылки") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    CallLog log;

    boost::signals2::scoped_connection second;
    picture.OnShapeAdded().connect([&](const shapes::IShape&) {
        log.Add("first");
        second.disconnect();
    });
    second = picture.OnShapeAdded().connect([&](const shapes::IShape&) { log.Add("second"); });
    picture.OnShapeAdded().connect([&](const shapes::IShape&) { log.Add("third"); });

    picture.AddShape(MakeCircleShape());

    REQUIRE(log.order == "firstthird");
}

TEST_CASE("Добавление обработчика во время рассылки начинает действовать только со следующего события") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int count1 = 0, count2 = 0;
    std::vector<boost::signals2::scoped_connection> subscriptions;

    boost::signals2::scoped_connection sub1 = picture.OnShapeAdded().connect([&](const shapes::IShape&) {
        count1++;
        if (count1 == 1) {
            subscriptions.emplace_back(picture.OnShapeAdded().connect([&](const shapes::IShape&) {
                count2++;
            }));
        }
    });

    picture.AddShape(MakeCircleShape());

    REQUIRE(count1 == 1);
    REQUIRE(count2 == 0);

    picture.AddShape(MakeRectangleShape());

    REQUIRE(count1 == 2);
    REQUIRE(count2 == 1);
}

TEST_CASE("Изменение Shape напрямую приводит к соответствующему событию Picture") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int moveCount = 0, colorCount = 0, geometryCount = 0;
    boost::signals2::scoped_connection sub1 = picture.OnShapeMoved().connect([&](const shapes::IShape&, double, double) { moveCount++; });
    boost::signals2::scoped_connection sub2 = picture.OnShapeColorChanged().connect([&](const shapes::IShape&, const gfx::Color&) { colorCount++; });
    boost::signals2::scoped_connection sub3 = picture.OnShapeGeometryChanged().connect([&](const shapes::IShape&, const shapes::IShapeGeometry&) { geometryCount++; });

    picture.AddShape(MakeCircleShape());

    REQUIRE(moveCount == 0);
    REQUIRE(colorCount == 0);
    REQUIRE(geometryCount == 0);

    auto& shape = picture.GetShape("1");
    shape.Move(10, 10);
    shape.SetColor(gfx::Color("#FF0000"));
    shape.SetGeometry(std::make_unique<shapes::Rectangle>(gfx::Rect{10, 10, 20, 20}));

    REQUIRE(moveCount == 1);
    REQUIRE(colorCount == 1);
    REQUIRE(geometryCount == 1);
}

TEST_CASE("Подписки переживают перевыделение памяти при добавлении фигур") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int colorCount = 0;
    std::string lastId;
    boost::signals2::scoped_connection sub = picture.OnShapeColorChanged().connect(
        [&](const shapes::IShape& shape, const gfx::Color&) {
            colorCount++;
            lastId = shape.GetId();
        }
    );

    constexpr int shapeCount = 50;
    for (int i = 0; i < shapeCount; ++i) {
        picture.AddShape(MakeCircleShape("shape" + std::to_string(i)));
    }
    REQUIRE(picture.GetShapesCount() == shapeCount);

    for (int i = 0; i < shapeCount; ++i) {
        picture.ChangeColor("shape" + std::to_string(i), gfx::Color("#FF0000"));
    }

    REQUIRE(colorCount == shapeCount);
    REQUIRE(lastId == "shape" + std::to_string(shapeCount - 1));
}

TEST_CASE("Перемещение всего рисунка порождает по одному событию перемещения на каждую фигуру") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    CallLog log;
    boost::signals2::scoped_connection sub = picture.OnShapeMoved().connect([&](const shapes::IShape& shape, double, double) {
        log.Add(shape.GetId());
    });

    picture.AddShape(MakeCircleShape());
    picture.AddShape(MakeRectangleShape());

    picture.MovePicture(5, 5);

    REQUIRE(log.order == "12");
}

TEST_CASE("Обработчик события удаления фигуры вызывается при удалении фигуры") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int removeCount = 0;
    shapes::IShape* removedShape = nullptr;
    boost::signals2::scoped_connection sub = picture.OnShapeRemoved().connect([&](const shapes::IShape& shape) {
        removeCount++;
        removedShape = const_cast<shapes::IShape*>(&shape);
    });

    picture.AddShape(MakeCircleShape());

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
    boost::signals2::scoped_connection sub = picture.OnShapeColorChanged().connect([&](const shapes::IShape&, const gfx::Color& color) {
        colorCount++;
        receivedColor = color;
    });

    picture.AddShape(MakeCircleShape());

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
    std::string receivedName;
    boost::signals2::scoped_connection sub = picture.OnShapeGeometryChanged().connect([&](const shapes::IShape&, const shapes::IShapeGeometry& geometry) {
        geomCount++;
        receivedName = geometry.GetShapeName();
    });

    picture.AddShape(MakeCircleShape());

    REQUIRE(geomCount == 0);
    picture.ChangeShape("1", std::make_unique<shapes::Rectangle>(gfx::Rect{100, 100, 50, 50}));
    REQUIRE(geomCount == 1);
    REQUIRE(receivedName == "rectangle");
}

TEST_CASE("После удаления фигуры её события больше не сообщают о себе Picture") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);

    int moveCount = 0, removedCount = 0;
    boost::signals2::scoped_connection sub1 = picture.OnShapeMoved().connect([&](const shapes::IShape&, double, double) { moveCount++; });
    boost::signals2::scoped_connection sub2 = picture.OnShapeRemoved().connect([&](const shapes::IShape&) { removedCount++; });

    picture.AddShape(MakeCircleShape());
    picture.MoveShape("1", 5, 5);
    REQUIRE(moveCount == 1);

    picture.DeleteShape("1");
    REQUIRE(removedCount == 1);

    picture.AddShape(MakeRectangleShape());
    picture.MoveShape("2", 5, 5);
    REQUIRE(moveCount == 2);
}

TEST_CASE("Разные экземпляры Picture имеют независимые наборы обработчиков") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture first(canvas);
    shapes::Picture second(canvas);

    int firstCount = 0, secondCount = 0;
    auto sub1 = first.OnShapeAdded().connect([&](const shapes::IShape&) { firstCount++; });

    second.AddShape(MakeCircleShape());

    REQUIRE(firstCount == 0);
    REQUIRE(secondCount == 0);

    first.AddShape(MakeCircleShape());
    REQUIRE(firstCount == 1);
    REQUIRE(secondCount == 0);
}

TEST_CASE("Копия Picture не наследует подписки внешних обработчиков") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture original(canvas);

    int originalCount = 0, copyCount = 0;
    auto sub = original.OnShapeAdded().connect([&](const shapes::IShape&) { originalCount++; });

    shapes::Picture copy(original);
    auto sub2 = copy.OnShapeAdded().connect([&](const shapes::IShape&) { copyCount++; });

    copy.AddShape(MakeCircleShape("copy1"));

    REQUIRE(copyCount == 1);
    REQUIRE(originalCount == 0);

    original.AddShape(MakeCircleShape());
    REQUIRE(originalCount == 1);
    REQUIRE(copyCount == 1);
}

TEST_CASE("Изменения фигуры в копии Picture не приводят к событиям оригинала") {
    std::ostringstream svg;
    gfx::CanvasSvg canvas(svg);
    shapes::Picture original(canvas);
    original.AddShape(MakeCircleShape());

    shapes::Picture copy(original);

    int copyMoveCount = 0;
    auto sub = copy.OnShapeMoved().connect([&](const shapes::IShape&, double, double) { copyMoveCount++; });

    copy.MoveShape("1", 3, 4);
    REQUIRE(copyMoveCount == 1);
}
