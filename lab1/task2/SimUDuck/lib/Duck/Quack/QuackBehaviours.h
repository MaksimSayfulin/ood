#ifndef QUACK_BEHAVIORS_H
#define QUACK_BEHAVIORS_H

#include <functional>
#include <iostream>

using QuackBehavior = std::function<void()>;

inline QuackBehavior MakeQuack()
{
    return [] {
        std::cout << "Quack Quack!" << std::endl;
    };
}

inline QuackBehavior MakeSqueak()
{
    return [] {
        std::cout << "Squeak!" << std::endl;
    };
}

inline QuackBehavior MakeMuteQuack()
{
    return [] {};
}

#endif