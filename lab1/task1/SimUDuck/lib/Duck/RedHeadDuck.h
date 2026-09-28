#ifndef REDHEADDUCK_H
#define REDHEADDUCK_H

#include "Duck.h"
#include "Fly/FlyWithWings.h"
#include "Quack/QuackBehavoiur.h"

#include <memory>

#include "Dance/Minuet.h"

class RedheadDuck : public Duck
{
public:
    RedheadDuck()
        : Duck(std::make_unique<FlyWithWings>(), std::make_unique<QuackBehavior>(), std::make_unique<Minuet>())
    {
    }

    void Display() const override
    {
        std::cout << "I'm redhead duck" << std::endl;
    }
};

#endif