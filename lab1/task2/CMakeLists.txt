cmake_minimum_required(VERSION 3.20)
project(task1 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

add_executable(sim_u_duck
        SimUDuck/SimUDuck.cpp
)

add_executable(unit_tests
        Test/Test.cpp
        SimUDuck/lib/Duck/Fly/FlyBehaviours.h
        SimUDuck/lib/Duck/Dance/DanceBehaviours.h
        SimUDuck/lib/Duck/Quack/QuackBehaviours.h
)

target_link_libraries(unit_tests
        PRIVATE
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
)