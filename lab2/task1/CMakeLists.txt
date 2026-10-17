cmake_minimum_required(VERSION 3.15)
project(task4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Boost.Signals2 используется как библиотека сигналов и слотов.
find_package(Boost 1.66 REQUIRED)

add_library(shape_graphics STATIC
        Shape/ShapeGeometry/IShapeGeometry.h
        Shape/IShape/IShape.h
        Shape/IShape/IShape.cpp
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
        Graphics/Canvas/ICanvas/ICanvas.cpp
        Graphics/Canvas/CanvasSVG/CanvasSvg.cpp
        Graphics/Canvas/CanvasSVG/CanvasSvg.h
        Picture/Picture.cpp
        Picture/Picture.h
)
target_include_directories(shape_graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(shape_graphics PUBLIC Boost::headers)

add_executable(task4
        main.cpp
        InterfaceIO/UserInput/UserInput.cpp
        InterfaceIO/UserInput/UserInput.h
        InterfaceIO/CommandParser/CommandParser.cpp
        InterfaceIO/CommandParser/CommandParser.h
        InterfaceIO/ConsolePictureObserver/ConsolePictureObserver.h
)
target_link_libraries(task4 PRIVATE shape_graphics)

add_executable(task4_tests
        Tests/ShapeTests.cpp
        Tests/ObserverTest.cpp
)
target_link_libraries(task4_tests PRIVATE shape_graphics)

enable_testing()
add_test(NAME task4_tests COMMAND task4_tests)
