#ifndef MODELDUCK_H
#define MODELDUCK_H

#include "Duck.h"
#include <iostream>

class ModelDuck : public Duck
{
public:
    ModelDuck()
        : Duck(MakeFlyNoWay(), MakeQuack(), MakeNoDance())
    {
    }

    void Display() const override
    {
        std::cout << "I'm model duck" << std::endl;
    }
};

#endif