#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../../Picture/Picture.h"

namespace io {
    class CommandParser {
    public:
        explicit CommandParser(shapes::Picture &picture);

        [[nodiscard]] bool Parse(const std::vector<std::string> &tokens) const;

    private:
        shapes::Picture &m_picture;
        static std::unique_ptr<shapes::IShapeGeometry> ParseGeometry(const std::vector<std::string> &tokens);
    };
}