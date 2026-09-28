#ifndef MALLARDDUCK_H
#define MALLARDDUCK_H

#include "Duck.h"
#include "Dance/Waltz.h"
#include "Fly/FlyWithWings.h"
#include "Quack/QuackBehavoiur.h"

class MallardDuck : public Duck
{
public:
    MallardDuck()
        : Duck(std::make_unique<FlyWithWings>(), std::make_unique<QuackBehavior>(), std::make_unique<Waltz>())
    {
    }

    void Display() const override
    {
        std::cout << "I'm mallard duck" << std::endl;
    }
};

#endif