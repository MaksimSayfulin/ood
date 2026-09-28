#ifndef QUACKBEHAVIOR_H
#define QUACKBEHAVIOR_H

#include <iostream>
#include "IQuackBehaviour.h"

class QuackBehavior : public IQuackBehavior
{
public:
    void Quack() override
    {
        std::cout << "Quack Quack!!!" << std::endl;
    }
};

#endif