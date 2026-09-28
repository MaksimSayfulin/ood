#ifndef MALLARD_DUCK_H
#define MALLARD_DUCK_H

#include "Duck.h"

class MallardDuck : public Duck
{
public:
    MallardDuck()
        : Duck(
            MakeFlyWithWings([this](size_t flightCount) {
                if (flightCount % 2 == 0)
                {
                    Quack();
                }
            }),
            MakeQuack(),
            MakeWaltz()
        )
    {}

    void Display() const override
    {
        std::cout << "I'm a real Mallard duck" << std::endl;
    }
};

#endif