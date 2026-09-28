#ifndef DANCE_BEHAVIORS_H
#define DANCE_BEHAVIORS_H

#include <functional>
#include <iostream>

using DanceBehavior = std::function<void()>;

inline DanceBehavior MakeMinuet()
{
    return [] {
        std::cout << "I'm dancing minuet!" << std::endl;
    };
}

inline DanceBehavior MakeWaltz()
{
    return [] {
        std::cout << "I'm dancing waltz!" << std::endl;
    };
}

inline DanceBehavior MakeNoDance()
{
    return [] {};
}

#endif