#ifndef FLYWITHWINGS_H
#define FLYWITHWINGS_H

#include "IFlyBehavior.h"
#include <functional>
#include <iostream>

class FlyWithWings : public IFlyBehavior
{
public:
    void Fly() override
    {
        m_countOfFlight++;

        if (m_onFlyCallback)
        {
            m_onFlyCallback(m_countOfFlight);
        }
    }

    void SetOnFlyCallback(std::function<void(size_t)> callback) override
    {
        m_onFlyCallback = std::move(callback);
    }

private:
    size_t m_countOfFlight = 0;
    std::function<void(size_t)> m_onFlyCallback;
};

#endif // FLYWITHWINGS_H