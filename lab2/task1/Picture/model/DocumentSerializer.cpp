#include "DocumentSerializer.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <format>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>

#include "../../Shape/model/ShapeFactory.h"

namespace {
    constexpr std::string_view fileHeader = "shapes";
    constexpr int fileVersion = 1;

    std::vector<std::string> Split(const std::string& line) {
        std::istringstream stream(line);
        std::vector<std::string> tokens;

        std::string token;
        while (stream >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }

    [[noreturn]] void Fail(int lineNumber, std::string_view reason) {
        throw shapes::DocumentError(std::format("Строка {}: {}", lineNumber, reason));
    }

    double ParseNumber(const std::string& token, int lineNumber, std::string_view name) {
        try {
            std::size_t parsed = 0;
            const double value = std::stod(token, &parsed);
            if (parsed == token.size()) {
                return value;
            }
        } catch (const std::exception&) {
        }

        Fail(lineNumber, std::format("'{}' не является числом ({})", token, name));
    }

    gfx::Color ParseColor(const std::string& token, int lineNumber) {
        const bool valid = token.size() == 7 && token.front() == '#'
            && std::all_of(token.begin() + 1, token.end(),
                           [](char c) { return std::isxdigit(static_cast<unsigned char>(c)) != 0; });

        if (!valid) {
            Fail(lineNumber, std::format("'{}' не является цветом вида #RRGGBB", token));
        }
        return gfx::Color{token};
    }
}

void shapes::DocumentSerializer::Save(const std::vector<IShape>& shapes, std::ostream& output) {
    if (!output) {
        throw DocumentError("Не удалось записать документ: поток недоступен");
    }

    output << fileHeader << ' ' << fileVersion << '\n';
    for (const auto& shape : shapes) {
        const auto bounds = shape.GetBounds();
        output << "shape " << shape.GetGeometry()->GetShapeName() << ' '
               << shape.GetId() << ' '
               << bounds.x << ' ' << bounds.y << ' ' << bounds.width << ' ' << bounds.height << ' '
               << shape.GetColor().GetColor() << '\n';
    }
}

void shapes::DocumentSerializer::SaveToFile(const std::vector<IShape>& shapes,
                                            const std::filesystem::path& path) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc );
    if (!file) {
        throw DocumentError(std::format("Не удалось открыть файл '{}' для записи", path.string()));
    }

    Save(shapes, file);

    file.close();
    if (!file) {
        throw DocumentError(std::format("Не удалось записать файл '{}'", path.string()));
    }
}

std::vector<shapes::IShape> shapes::DocumentSerializer::Load(std::istream& input) {
    if (!input) {
        throw DocumentError("Не удалось прочитать документ: файл недоступен");
    }

    std::vector<IShape> shapes;
    std::unordered_set<std::string> identifiers;
    bool headerRead = false;
    std::string line;
    int lineNumber = 0;

    while (std::getline(input, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        const auto tokens = Split(line);
        if (tokens.empty() || tokens.front().starts_with('#')) {
            continue;
        }

        if (!headerRead) {
            if (tokens.size() != 2 || tokens[0] != fileHeader) {
                Fail(lineNumber, "файл не является документом редактора");
            }
            if (ParseNumber(tokens[1], lineNumber, "версия") != fileVersion) {
                Fail(lineNumber, std::format("неподдерживаемая версия '{}'", tokens[1]));
            }
            headerRead = true;
            continue;
        }

        if (tokens.front() != "shape") {
            Fail(lineNumber, std::format("неизвестная команда '{}'", tokens.front()));
        }
        if (tokens.size() != 8) {
            Fail(lineNumber, "фигура должна задаваться 7 параметрами: тип, id, x, y, ширина, высота, цвет");
        }

        const auto& type = tokens[1];
        const auto& id = tokens[2];
        if (!ShapeFactory::IsSupported(type)) {
            Fail(lineNumber, std::format("тип фигуры '{}' не поддерживается", type));
        }
        if (!identifiers.insert(id).second) {
            Fail(lineNumber, std::format("фигура с идентификатором '{}' уже есть в документе", id));
        }

        const gfx::Rect bounds{
            ParseNumber(tokens[3], lineNumber, "x"),
            ParseNumber(tokens[4], lineNumber, "y"),
            ParseNumber(tokens[5], lineNumber, "ширина"),
            ParseNumber(tokens[6], lineNumber, "высота")
        };
        if (!bounds.IsValid()) {
            Fail(lineNumber, "размеры фигуры не могут быть отрицательными");
        }

        shapes.emplace_back(id, ParseColor(tokens[7], lineNumber),
                              ShapeFactory::CreateGeometry(type, bounds));
    }

    if (!headerRead) {
        throw DocumentError("Файл пуст или не содержит заголовка документа");
    }

    return shapes;
}

std::vector<shapes::IShape> shapes::DocumentSerializer::LoadFromFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw DocumentError(std::format("Не удалось открыть файл '{}'", path.string()));
    }

    return Load(file);
}