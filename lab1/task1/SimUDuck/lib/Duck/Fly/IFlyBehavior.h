#ifndef LAB01_SIMUDUCK_IFLYBEHAVIOR_H
#define LAB01_SIMUDUCK_IFLYBEHAVIOR_H
#include <functional>

struct IFlyBehavior
{
    virtual ~IFlyBehavior() = default;
    virtual void Fly() = 0;

    virtual void SetOnFlyCallback(std::function<void(size_t flightCount)> callback) {
        m_onFlyCallback = std::move(callback);
    }

protected:
    void NotifyFly(const size_t flightCount) const {
        if (m_onFlyCallback) {
            m_onFlyCallback(flightCount);
        }
    }

    unsigned int m_countOfFlight = 0;

private:
    std::function<void(size_t flightCount)> m_onFlyCallback;
};

#endif