#ifndef MUTEQUAKBEHAVIOR_H
#define MUTEQUAKBEHAVIOR_H

#include "QuackBehavoiur.h"

class MuteQuackBehavior : public IQuackBehavior
{
public:
    void Quack() override {}
};

#endif