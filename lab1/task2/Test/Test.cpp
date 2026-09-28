#include <gtest/gtest.h>
#include "../SimUDuck/lib/Duck/Duck.h"

class TestDuck : public Duck
{
public:
    TestDuck(FlyBehavior fly, QuackBehavior quack, DanceBehavior dance)
        : Duck(std::move(fly), std::move(quack), std::move(dance)) {}

    void Display() const override {}
};

TEST(FunctionalDuckTest, QuacksOnEverySecondFlightWithMutableClosure)
{
    size_t quackCount = 0;

    QuackBehavior quackBehavior = [&quackCount] { ++quackCount; };

    TestDuck* duckPtr = nullptr;
    auto flyBehavior = MakeFlyWithWings([&duckPtr](size_t flightCount) {
        if (flightCount % 2 == 0 && duckPtr)
        {
            duckPtr->Quack();
        }
        std::cout << "I'm flying with wings " << flightCount << " times!!" << std::endl;

    });

    TestDuck duck(std::move(flyBehavior), quackBehavior, MakeNoDance());
    duckPtr = &duck;

    duck.Fly();
    EXPECT_EQ(quackCount, 0);

    duck.Fly();
    EXPECT_EQ(quackCount, 1);

    duck.Fly();
    EXPECT_EQ(quackCount, 1);

    duck.Fly();
    EXPECT_EQ(quackCount, 2);
}