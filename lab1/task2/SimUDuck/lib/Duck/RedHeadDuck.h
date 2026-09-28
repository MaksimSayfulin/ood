#ifndef REDHEADDUCK_H
#define REDHEADDUCK_H

#include "Duck.h"
#include <iostream>

class RedheadDuck : public Duck
{
public:
    RedheadDuck()
        : Duck(MakeFlyWithWings(), MakeQuack(), MakeMinuet())
    {
    }

    void Display() const override
    {
        std::cout << "I'm redhead duck" << std::endl;
    }
};

#endif