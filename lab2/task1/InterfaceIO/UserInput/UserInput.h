#pragma once
#include <iostream>
#include <istream>
#include <vector>

namespace io {
    class UserInput {
    public:
        explicit UserInput(std::istream& input = std::cin);

        [[nodiscard]] std::vector<std::string> ReadInput() const;

    private:
        std::istream& m_input;
    };
}