#ifndef FLY_BEHAVIORS_H
#define FLY_BEHAVIORS_H

#include <functional>
#include <iostream>

using FlyBehavior = std::function<void()>;
using OnFlyCallback = std::function<void(size_t flightCount)>;

inline FlyBehavior MakeFlyWithWings(OnFlyCallback onFly = nullptr)
{
    return [flightCount = size_t{0}, onFly = std::move(onFly)]() mutable {
        flightCount++;
        if (onFly)
        {
            onFly(flightCount);
        }
    };
}

inline FlyBehavior MakeFlyNoWay()
{
    return [] {};
}

#endif