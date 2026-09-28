#ifndef DUCK_H
#define DUCK_H

#include "Dance/DanceBehaviours.h"
#include "Fly/FlyBehaviours.h"
#include "Quack/QuackBehaviours.h"

#include <cassert>
#include <iostream>
#include <utility>

#include "Dance/DanceBehaviours.h"
#include "Fly/FlyBehaviours.h"
#include "Quack/QuackBehaviours.h"

class Duck
{
public:
    Duck(FlyBehavior flyBehavior,
         QuackBehavior quackBehavior,
         DanceBehavior danceBehavior)
        : m_quackBehavior(std::move(quackBehavior))
        , m_danceBehavior(std::move(danceBehavior))
    {
        assert(m_quackBehavior);
        assert(m_danceBehavior);
        SetFlyBehavior(std::move(flyBehavior));
    }

    virtual ~Duck() = default;

    void Quack() const
    {
        if (m_quackBehavior)
        {
            m_quackBehavior();
        }
    }

    void Swim() const
    {
        std::cout << "I'm swimming" << std::endl;
    }

    void Fly()
    {
        if (m_flyBehavior)
        {
            m_flyBehavior();
        }
    }

    void Dance() const
    {
        if (m_danceBehavior)
        {
            m_danceBehavior();
        }
    }

    void SetFlyBehavior(FlyBehavior flyBehavior)
    {
        assert(flyBehavior);
        m_flyBehavior = std::move(flyBehavior);
    }

    virtual void Display() const = 0;

private:
    FlyBehavior m_flyBehavior;
    QuackBehavior m_quackBehavior;
    DanceBehavior m_danceBehavior;
};

#endif