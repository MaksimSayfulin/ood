#include "UserInput.h"

#include <sstream>
#include <string>

namespace {
    std::vector<std::string> ParseString(const std::string& line) {
        std::vector<std::string> tokens;
        std::istringstream iss(line);
        std::string token;

        while (iss >> token) {
            tokens.push_back(token);
        }

        return tokens;
    }
}

io::UserInput::UserInput(std::istream& input): m_input(input) {
    if (!m_input) {
        throw std::invalid_argument("No input stream");
    }
}

std::vector<std::string> io::UserInput::ReadInput() const {
    std::string line;
    std::getline(m_input, line);
    return ParseString(line);
}
