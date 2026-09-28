cmake_minimum_required(VERSION 3.15)
project(task4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(task4 main.cpp
        Shape/ShapeGeometry/IShapeGeometry.h
        Tests/ShapeTests.cpp
        Shape/IShape/IShape.h
        Shape/ShapeGeometry/ShapeGeometries/Circle/Circle.cpp
        Shape/ShapeGeometry/ShapeGeometries/Circle/Circle.h
        Shape/ShapeGeometry/ShapeGeometries/Rectangle/Rectangle.cpp
        Shape/ShapeGeometry/ShapeGeometries/Rectangle/Rectangle.h
        Shape/ShapeGeometry/ShapeGeometries/Triangle/Triangle.h
        Shape/ShapeGeometry/ShapeGeometries/Triangle/Triangle.cpp
        Shape/ShapeGeometry/ShapeGeometries/Line/Line.cpp
        Shape/ShapeGeometry/ShapeGeometries/Line/Line.h
        Shape/ShapeGeometry/ShapeGeometries/Text/Text.cpp
        Shape/ShapeGeometry/ShapeGeometries/Text/Text.h
        Graphics/Color/Color.h
        Graphics/Color/Color.cpp
        Graphics/Canvas/ICanvas/ICanvas.h
        Graphics/Canvas/CanvasSVG/CanvasSvg.cpp
        Graphics/Canvas/CanvasSVG/CanvasSvg.h
        Graphics/Canvas/ICanvas/ICanvas.cpp
        Shape/IShape/IShape.cpp
        Picture/Picture.cpp
        Picture/Picture.h
        InterfaceIO/UserInput/UserInput.cpp
        InterfaceIO/UserInput/UserInput.h
        InterfaceIO/CommandParser/CommandParser.cpp
        InterfaceIO/CommandParser/CommandParser.h)
