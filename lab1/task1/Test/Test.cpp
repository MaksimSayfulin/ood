#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../SimUDuck/lib/DuckFunctions.h"
#include "../SimUDuck/lib/Duck/Dance/IDanceBehavior.h"
#include "../SimUDuck/lib/Duck/Dance/NoDance.h"
#include "../SimUDuck/lib/Duck/Fly/FlyNoWay.h"
#include "../SimUDuck/lib/Duck/Fly/FlyWithWings.h"
#include "../SimUDuck/lib/Duck/Fly/IFlyBehavior.h"
#include "../SimUDuck/lib/Duck/Quack/IQuackBehaviour.h"
#include "../SimUDuck/lib/Duck/Quack/MuteQuackBehaviour.h"

using ::testing::Mock;
using ::testing::InSequence;

class MockDanceBehavior : public IDanceBehavior {
public:
    MOCK_METHOD(void, Dance, (), (override));
};

class MockQuackBehavior : public IQuackBehavior {
public:
    MOCK_METHOD(void, Quack, (), (override));
};

class TestDuck : public Duck { // можно без тестовой утки
public:
    TestDuck(std::unique_ptr<IFlyBehavior> fly,
             std::unique_ptr<IQuackBehavior> quack,
             std::unique_ptr<IDanceBehavior> dance)
        : Duck(std::move(fly), std::move(quack), std::move(dance)) {}

    void Display() const override {}
};

TEST(DuckDanceTest, DanceDelegatesToDanceBehavior) {
    auto mockDance = std::make_unique<MockDanceBehavior>();
    MockDanceBehavior* dancePtr = mockDance.get();

    EXPECT_CALL(*dancePtr, Dance()).Times(1);

    TestDuck duck(std::make_unique<FlyNoWay>(),
                  std::make_unique<MuteQuackBehavior>(),
                  std::move(mockDance));

    duck.Dance();
}

TEST(DuckFlightReacztionTest, QuacksExactlyOnEverySecondFlight) {
    auto mockQuack = std::make_unique<MockQuackBehavior>();
    MockQuackBehavior* quackPtr = mockQuack.get();

    TestDuck duck(std::make_unique<FlyWithWings>(), std::move(mockQuack), std::make_unique<NoDance>());

    EXPECT_CALL(*quackPtr, Quack()).Times(0);
    duck.Fly();

    EXPECT_CALL(*quackPtr, Quack()).Times(1);
    duck.Fly();

    EXPECT_CALL(*quackPtr, Quack()).Times(0);
    duck.Fly();

    EXPECT_CALL(*quackPtr, Quack()).Times(1);
    duck.Fly();
}

TEST(DuckFlightReactionTest, RebindsCallbackOnSetFlyBehavior) {
    auto mockQuack = std::make_unique<MockQuackBehavior>();
    MockQuackBehavior* quackPtr = mockQuack.get();

    TestDuck duck(std::make_unique<FlyNoWay>(), std::move(mockQuack), std::make_unique<NoDance>());

    EXPECT_CALL(*quackPtr, Quack()).Times(0);
    duck.Fly();

    duck.SetFlyBehavior(std::make_unique<FlyWithWings>());

    duck.Fly();

    EXPECT_CALL(*quackPtr, Quack()).Times(1);
    duck.Fly();
}