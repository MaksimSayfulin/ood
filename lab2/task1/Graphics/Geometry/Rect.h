#pragma once

namespace gfx {
    struct Point {
        double x = 0;
        double y = 0;

        bool operator==(const Point&) const = default;
    };

    struct Size {
        double width = 0;
        double height = 0;

        bool operator==(const Size&) const = default;
    };

    struct Rect {
        double x = 0;
        double y = 0;
        double width = 0;
        double height = 0;

        [[nodiscard]] double Right() const { return x + width; }
        [[nodiscard]] double Bottom() const { return y + height; }
        [[nodiscard]] Point Center() const { return {x + width / 2, y + height / 2}; }

        [[nodiscard]] bool Contains(const Point& point) const {
            return point.x >= x && point.x <= Right()
                && point.y >= y && point.y <= Bottom();
        }

        [[nodiscard]] bool IsValid() const { return width >= 0 && height >= 0; }

        bool operator==(const Rect&) const = default;
    };
}