#ifndef DECOYDUCK_H
#define DECOYDUCK_H

#include "Duck.h"
#include <iostream>

class DecoyDuck : public Duck
{
public:
    DecoyDuck()
        : Duck(MakeFlyNoWay(), MakeMuteQuack(), MakeNoDance())
    {
    }

    void Display() const override
    {
        std::cout << "I'm decoy duck" << std::endl;
    }
};

#endif