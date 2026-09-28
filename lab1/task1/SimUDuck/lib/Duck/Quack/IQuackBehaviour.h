#ifndef IQUACKBEHAVIOR_H
#define IQUACKBEHAVIOR_H

struct IQuackBehavior
{
    virtual ~IQuackBehavior()= default;
    virtual void Quack() = 0;
};

#endif