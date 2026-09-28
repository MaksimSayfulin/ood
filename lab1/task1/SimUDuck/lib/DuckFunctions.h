#ifndef DUCKFUNCTIONS_H
#define DUCKFUNCTIONS_H

#include "Duck/Duck.h"

inline void DrawDuck(Duck const& duck)
{
    duck.Display();
}

inline void PlayWithDuck(Duck& duck)
{
    DrawDuck(duck);
    duck.Quack();
    duck.Fly();
    duck.Dance();
    std::cout << std::endl;
}

#endif