#include "../Catch/Catch.hpp"
#include "../Editor/EditorLayout.h"

namespace {
    gfx::Point CenterOf(const editor::layout::ToolbarButton& button) {
        return {button.bounds.x + button.bounds.width / 2,
                button.bounds.y + button.bounds.height / 2};
    }
}

TEST_CASE("Раскладка: попадание в кнопку панели инструментов") {
    const auto& buttons = editor::layout::Toolbar();
    REQUIRE(buttons.size() >= 5);

    SECTION("Центр каждой кнопки попадает в эту же кнопку") {
        for (const auto& button : buttons) {
            REQUIRE(editor::layout::HitTestToolbar(CenterOf(button)) == button.command);
        }
    }

    SECTION("Углы кнопки тоже принадлежат кнопке") {
        for (const auto& button : buttons) {
            const auto topLeft = gfx::Point{button.bounds.x + 1, button.bounds.y + 1};
            const auto bottomRight = gfx::Point{button.bounds.Right() - 1, button.bounds.Bottom() - 1};

            REQUIRE(editor::layout::HitTestToolbar(topLeft) == button.command);
            REQUIRE(editor::layout::HitTestToolbar(bottomRight) == button.command);
        }
    }

    SECTION("Панель занимает только свою полосу, холст в неё не попадает") {
        for (const auto& button : buttons) {
            REQUIRE(button.bounds.y >= 0);
            REQUIRE(button.bounds.Bottom() <= editor::layout::toolbarHeight);
        }

        const auto canvas = editor::layout::CanvasBounds({1280, 800});
        const auto insideCanvas = gfx::Point{canvas.x + 10, canvas.y + 10};
        REQUIRE_FALSE(editor::layout::HitTestToolbar(insideCanvas).has_value());
    }

    SECTION("Пустая область рядом с кнопками не считается нажатием") {
        REQUIRE_FALSE(editor::layout::HitTestToolbar({0, 0}).has_value());
        REQUIRE_FALSE(editor::layout::HitTestToolbar({1279, 22}).has_value());
    }

    SECTION("Кнопки не наезжают друг на друга") {
        for (std::size_t i = 1; i < buttons.size(); ++i) {
            REQUIRE(buttons[i - 1].bounds.Right() <= buttons[i].bounds.x);
        }
    }
}

TEST_CASE("Раскладка: холст и строка состояния не пересекаются") {
    const gfx::Size window{1280, 800};
    const auto canvas = editor::layout::CanvasBounds(window);
    const auto status = editor::layout::StatusBarBounds(window);

    REQUIRE(canvas.y >= editor::layout::toolbarHeight);
    REQUIRE(canvas.Bottom() <= status.y);
    REQUIRE(status.y + status.height == window.height);
}

TEST_CASE("Раскладка: окно открывается по центру экрана") {
    SECTION("На экране больше окна - окно центрируется") {
        const auto origin = editor::layout::WindowOrigin({1280, 800}, {2560, 1600});

        REQUIRE(origin.x == 640);
        REQUIRE(origin.y == 400);
    }

    SECTION("Окно целиком помещается в экран") {
        const gfx::Size window{1280, 800};
        const gfx::Size screen{2560, 1600};
        const auto origin = editor::layout::WindowOrigin(window, screen);

        REQUIRE(origin.x >= 0);
        REQUIRE(origin.y >= 0);
        REQUIRE(origin.x + window.width <= screen.width);
        REQUIRE(origin.y + window.height <= screen.height);
    }

    SECTION("Если окно больше экрана, оно прижимается к левому верхнему углу") {
        REQUIRE(editor::layout::WindowOrigin({1600, 1000}, {1280, 800})
                == (gfx::Point{0, 0}));
    }
}

TEST_CASE("Раскладка: размер окна подстраивается под экран") {
    SECTION("На экране 2560x1600 окно занимает почти весь экран") {
        const auto size = editor::layout::InitialWindowSize({2560, 1600});

        REQUIRE(size.width == Approx(2560 * editor::layout::desktopShare).margin(1));
        REQUIRE(size.height == Approx(1600 * editor::layout::desktopShare).margin(1));
        REQUIRE(size.width <= 2560);
        REQUIRE(size.height <= 1600);
    }

    SECTION("Холст не меньше размера окна за вычетом панелей") {
        const gfx::Size screen{2560, 1600};
        const auto window = editor::layout::InitialWindowSize(screen);
        const auto canvas = editor::layout::CanvasBounds(window);

        REQUIRE(canvas.width > 2200);
        REQUIRE(canvas.height > 1400);
    }

    SECTION("Окно не выходит за пределы экрана") {
        const auto size = editor::layout::InitialWindowSize({900, 600});

        REQUIRE(size.width <= 900);
        REQUIRE(size.height <= 600);
    }

    SECTION("На маленьком экране размер окна всё равно не больше экрана") {
        const auto size = editor::layout::InitialWindowSize({1024, 700});

        REQUIRE(size.width <= 1024);
        REQUIRE(size.height <= 700);
        REQUIRE(size.width >= 900);
    }
}

TEST_CASE("Раскладка: холст и панель слоёв занимают окно и не пересекаются") {
    const gfx::Size screen{2560, 1600};
    const auto window = editor::layout::InitialWindowSize(screen);
    const auto origin = editor::layout::WindowOrigin(window, screen);
    const auto canvas = editor::layout::CanvasBounds(window);
    const auto list = editor::layout::ShapeListBounds(window);

    SECTION("Панель слоёв находится справа от холста") {
        REQUIRE(list.x >= canvas.Right());
        REQUIRE(list.width > 0);
        REQUIRE(list.height > 0);
    }

    SECTION("Панель слоёв прижата к правому краю окна") {
        const double right = origin.x + list.Right();

        REQUIRE(right == Approx(origin.x + window.width - editor::layout::canvasMargin).margin(1));
    }

    SECTION("Строки списка не выходят за пределы панели") {
        const auto rows = editor::layout::ShapeListRowsBounds(window);

        REQUIRE(rows.x >= list.x);
        REQUIRE(rows.Right() <= list.Right());
        REQUIRE(rows.y >= list.y);
        REQUIRE(rows.Bottom() <= list.Bottom());
    }

    SECTION("Центр панели слоёв смещён вправо относительно центра холста") {
        const double canvasCenter = origin.x + canvas.x + canvas.width / 2;
        const double listCenter = origin.x + list.x + list.width / 2;

        REQUIRE(listCenter > canvasCenter);
        REQUIRE(listCenter == Approx(origin.x + window.width - editor::layout::canvasMargin
                                    - editor::layout::shapeListWidth / 2)
                                 .margin(1));
    }

    SECTION("По вертикали холст и панель совпадают") {
        REQUIRE(list.y == canvas.y);
        REQUIRE(list.height == canvas.height);
    }
}