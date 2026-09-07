#include "CommandParser.h"

#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>

#include "../../Shape/ShapeGeometry/ShapeGeometries/Circle/Circle.h"
#include "../../Shape/ShapeGeometry/ShapeGeometries/Line/Line.h"
#include "../../Shape/ShapeGeometry/ShapeGeometries/Rectangle/Rectangle.h"
#include "../../Shape/ShapeGeometry/ShapeGeometries/Text/Text.h"
#include "../../Shape/ShapeGeometry/ShapeGeometries/Triangle/Triangle.h"

io::CommandParser::CommandParser(shapes::Picture &picture): m_picture(picture) {

}

bool io::CommandParser::Parse(const std::vector<std::string> &tokens) const {
    if (tokens.empty()) {
        return true;
    }

    const std::string& command = tokens[0];
    if (command == "AddShape") {
        m_picture.AddShape(tokens[1], gfx::Color(tokens[2]), ParseGeometry(tokens));
    } else if (command == "MoveShape") {
        m_picture.MoveShape(tokens[1], std::stod(tokens[2]), std::stod(tokens[3]));
    } else if (command == "MovePicture") {
        m_picture.MovePicture(std::stod(tokens[2]), std::stod(tokens[3]));
    } else if (command == "DeleteShape") {
        m_picture.DeleteShape(tokens[1]);
    } else if (command == "List") {
        m_picture.List();
    } else if (command == "ChangeColor") {
        m_picture.ChangeColor(tokens[1], gfx::Color(tokens[2]));
    } else if (command == "ChangeShape") {
        m_picture.ChangeShape(tokens[1], ParseGeometry(tokens));
    } else if (command == "DrawShape") {
        m_picture.DrawShape(tokens[1]);
    } else if (command == "DrawPicture") {
        m_picture.DrawPicture();
    } else if (command == "exit") {
        return false;
    } else {
        std::cout << "Unknown command: " << command << std::endl;
    }

    return true;
}

std::unique_ptr<shapes::IShapeGeometry> io::CommandParser::ParseGeometry(const std::vector<std::string> &tokens) {


    const std::string& geometryType = tokens[3];

    if (geometryType == "circle") {
        if (tokens.size() < 7) throw std::invalid_argument("Too few tokens for circle");
        return std::make_unique<shapes::Circle>(std::stod(tokens[4]), std::stod(tokens[5]), std::stod(tokens[6]));
    }
    if (geometryType == "rectangle") {
        if (tokens.size() < 8) throw std::invalid_argument("Too few tokens for rectangle");
        return std::make_unique<shapes::Rectangle>(std::stod(tokens[4]), std::stod(tokens[5]), std::stod(tokens[6]), std::stod(tokens[7]));
    }
    if (geometryType == "triangle") {
        if (tokens.size() < 10) throw std::invalid_argument("Too few tokens for triangle");
        return std::make_unique<shapes::Triangle>(std::stod(tokens[4]), std::stod(tokens[5]), std::stod(tokens[6]), std::stod(tokens[7]), std::stod(tokens[8]), std::stod(tokens[9]));
    }
    if (geometryType == "line") {
        if (tokens.size() < 8) throw std::invalid_argument("Too few tokens for line");
        return std::make_unique<shapes::Line>(std::stod(tokens[4]), std::stod(tokens[5]), std::stod(tokens[6]), std::stod(tokens[7]));
    }
    if (geometryType == "text") {
        if (tokens.size() < 8) throw std::invalid_argument("Too few tokens for text");
        return std::make_unique<shapes::Text>(std::stod(tokens[4]), std::stod(tokens[5]), std::stod(tokens[6]), tokens[7].data());
    }

    throw std::invalid_argument("Invalid geometry type: " + geometryType);
}


