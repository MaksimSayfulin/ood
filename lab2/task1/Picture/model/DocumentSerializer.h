#pragma once

#include <filesystem>
#include <stdexcept>
#include <vector>

#include "../../Shape/model/IShape/IShape.h"

namespace shapes {
    class DocumentError : public std::runtime_error {
    public:
        explicit DocumentError(const std::string& message) : std::runtime_error(message) {}
    };

    class DocumentSerializer {
    public:
        static void Save(const std::vector<IShape>& shapes, std::ostream& output);
        static void SaveToFile(const std::vector<IShape>& shapes, const std::filesystem::path& path);

        static std::vector<IShape> Load(std::istream& input);
        static std::vector<IShape> LoadFromFile(const std::filesystem::path& path);
    };
}