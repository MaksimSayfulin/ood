cmake_minimum_required(VERSION 3.25)
project(task4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost 1.66 REQUIRED)
find_package(SFML 3 COMPONENTS Graphics REQUIRED)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>)

add_library(core STATIC
        Graphics/Geometry/Rect.h
        Shape/model/ShapeFactory.h
        Shape/model/ShapeFactory.cpp
        Shape/model/IShape/IShape.h
        Shape/model/IShape/IShape.cpp
        Shape/model/ShapeGeometry/IShapeGeometry.h
        Shape/model/ShapeGeometry/ShapeGeometries/Rectangle/Rectangle.h
        Shape/model/ShapeGeometry/ShapeGeometries/Rectangle/Rectangle.cpp
        Shape/model/ShapeGeometry/ShapeGeometries/Ellipse/Ellipse.h
        Shape/model/ShapeGeometry/ShapeGeometries/Ellipse/Ellipse.cpp
        Shape/model/ShapeGeometry/ShapeGeometries/Triangle/Triangle.h
        Shape/model/ShapeGeometry/ShapeGeometries/Triangle/Triangle.cpp
        Shape/model/ShapeGeometry/ShapeGeometries/Line/Line.h
        Shape/model/ShapeGeometry/ShapeGeometries/Line/Line.cpp
        Graphics/Color/Color.h
        Graphics/Color/Color.cpp
        Graphics/Canvas/ICanvas/ICanvas.h
        Graphics/Canvas/ICanvas/ICanvas.cpp
        Graphics/Canvas/CanvasSVG/CanvasSvg.h
        Graphics/Canvas/CanvasSVG/CanvasSvg.cpp
        Picture/model/Picture.h
        Picture/model/Picture.cpp
        Picture/model/DocumentSerializer.h
        Picture/model/DocumentSerializer.cpp
)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(core
        PUBLIC  Boost::headers
        PRIVATE project_warnings)

add_library(editor STATIC
        Editor/EditorTypes.h
        Editor/FilePrompt.h
        Editor/FilePrompt.cpp
        Editor/SelectionFrame.h
        Editor/SelectionFrame.cpp
        Editor/SnapEngine.h
        Editor/SnapEngine.cpp
        Editor/EditorLayout.h
        Editor/EditorLayout.cpp
        Editor/EditorController.h
        Editor/EditorController.cpp
)
target_link_libraries(editor
        PUBLIC  core
        PRIVATE project_warnings)

add_library(frontend_sfml STATIC
        Picture/view/SfmlCanvas.h
        Picture/view/SfmlCanvas.cpp
        Picture/view/SfmlColor.h
        Picture/view/SfmlColor.cpp
        Editor/EditorWindow.h
        Editor/EditorWindow.cpp
        Editor/EditorView.h
        Editor/EditorView.cpp
        Editor/EditorSession.h
        Editor/EditorSession.cpp
        Editor/EditorApp.h
        Editor/EditorApp.cpp
)
target_link_libraries(frontend_sfml
        PUBLIC  editor SFML::Graphics
        PRIVATE project_warnings)

add_executable(task4 main.cpp)
target_link_libraries(task4
        PRIVATE frontend_sfml project_warnings)

enable_testing()

add_executable(task4_tests
        Tests/ShapeTests.cpp
        Tests/ObserverTest.cpp
        Tests/DocumentTests.cpp
        Tests/EditorControllerTest.cpp
        Tests/EditorLayoutTest.cpp
        Tests/FilePromptTest.cpp
        Tests/MultiSelectionTest.cpp
        Tests/ShapeListTest.cpp
        Tests/SnapTest.cpp
)
target_link_libraries(task4_tests PRIVATE core editor project_warnings)
add_test(NAME task4_tests COMMAND task4_tests)