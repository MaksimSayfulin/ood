#ifndef DUCK_H
#define DUCK_H

#include "Fly/IFlyBehavior.h"
#include "Dance/IDanceBehavior.h"

#include <cassert>
#include <iostream>
#include <memory>

#include "Quack/IQuackBehaviour.h"

class Duck
{
public:
    Duck(std::unique_ptr<IFlyBehavior> flyBehavior,
         std::unique_ptr<IQuackBehavior> quackBehavior,
         std::unique_ptr<IDanceBehavior> danceBehavior)
        : m_quackBehavior(std::move(quackBehavior))
        , m_danceBehavior(std::move(danceBehavior))
    {
        assert(m_quackBehavior);
        assert(m_danceBehavior);
        SetFlyBehavior(std::move(flyBehavior));
    }

    void Quack() const
    {
        m_quackBehavior->Quack();
    }

    void Swim() const
    {
        std::cout << "I'm swimming" << std::endl;
    }

    void Fly()
    {
        if (m_flyBehavior)
        {
            m_flyBehavior->Fly();
        }
    }

    void Dance() const
    {
        m_danceBehavior->Dance();
    }

    void SetFlyBehavior(std::unique_ptr<IFlyBehavior> flyBehavior)
    {
        assert(flyBehavior);
        m_flyBehavior = std::move(flyBehavior);

        SetupFlightReaction();
    }

    virtual void Display() const = 0;
    virtual ~Duck() = default;

private:
    std::unique_ptr<IFlyBehavior> m_flyBehavior;
    std::unique_ptr<IQuackBehavior> m_quackBehavior;
    std::unique_ptr<IDanceBehavior> m_danceBehavior;

    void SetupFlightReaction()
    {
        if (!m_flyBehavior) {
            return;
        }

        m_flyBehavior->SetOnFlyCallback([this](const size_t flightCount) {
            if (flightCount % 2 == 0) {
                Quack();
            }
            std::cout << "I'm flying with wings " << flightCount << " times!!" << std::endl;

        });
    }
};

#endif