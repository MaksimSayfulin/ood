#ifndef MODELDUCK_H
#define MODELDUCK_H

#include "Duck.h"
#include "Fly/FlyNoWay.h"
#include "Quack/QuackBehavoiur.h"

class ModelDuck : public Duck
{
public:
    ModelDuck()
        : Duck(std::make_unique<FlyNoWay>(), std::make_unique<QuackBehavior>(), std::make_unique<NoDance>())
    {
    }

    void Display() const override
    {
        std::cout << "I'm model duck" << std::endl;
    }
};

#endif