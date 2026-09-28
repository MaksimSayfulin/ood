cmake_minimum_required(VERSION 3.20)
project(task1 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# В Arch Linux и GTest >= 1.14 GMock входит в состав пакета GTest
find_package(GTest REQUIRED)

# ==============================================================================
# 1. Основное приложение (запуск программы SimUDuck)
# ==============================================================================
add_executable(sim_u_duck
        SimUDuck/SimUDuck.cpp
)

# ==============================================================================
# 2. Исполняемый файл для модульных тестов
# ==============================================================================
add_executable(unit_tests
        Test/Test.cpp
)

# Подключение заголовочных файлов и линковка GoogleTest/GoogleMock к тестам
target_link_libraries(unit_tests
        PRIVATE
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)