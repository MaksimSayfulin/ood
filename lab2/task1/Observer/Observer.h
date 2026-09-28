#pragma once

#include <algorithm>
#include <atomic>
#include <memory>
#include <set>
#include <vector>

template <typename T>
class IObserver
{
public:
    virtual void Update(T const& data) = 0;
    virtual ~IObserver() = default;
};

template <typename T>
class IObservable
{
public:
    virtual ~IObservable() = default;
    virtual void RegisterObserver(IObserver<T> & observer) = 0;
    virtual void NotifyObservers() = 0;
    virtual void RemoveObserver(IObserver<T> & observer) = 0;
};

template <class T>
class CObservable;

template <class T>
struct SubscriptionState {
    bool alive{ true }; // не нужно atomic
    CObservable<T>* observable;
};

template <class T>
class Subscription {
public:
    Subscription() = default;

    ~Subscription() {
        Disconnect();
    }

    void Disconnect() {
        if (m_state && m_state->alive) {
            m_state->observable->RemoveObserver(*m_observer);
        }
        m_state.reset();
        m_observer = nullptr;
    }

    Subscription(Subscription&& other) noexcept
        : m_state(std::move(other.m_state)), m_observer(other.m_observer) {
        other.m_state.reset();
        other.m_observer = nullptr;
    }

    Subscription& operator=(Subscription&& other) noexcept {
        if (this != &other) {
            Disconnect();
            m_state = std::move(other.m_state);
            m_observer = other.m_observer;
            other.m_state.reset();
            other.m_observer = nullptr;
        }
        return *this;
    }

private:
    std::shared_ptr<SubscriptionState<T>> m_state;
    IObserver<T>* m_observer = nullptr;
};

template <class T>
class CObservable : public IObservable<T>
{
public:
    typedef IObserver<T> ObserverType;

    CObservable() : m_state(std::make_shared<SubscriptionState<T>>()) {
        m_state->observable = this;
    }
    
    CObservable(const CObservable&) : m_observers(), m_activeObservers(), m_state(std::make_shared<SubscriptionState<T>>()) {
        m_state->observable = this;
    }
    
    ~CObservable() override {
        m_state->alive = false;
    }

    Subscription<T> Subscribe(ObserverType& observer) {
        RegisterObserver(observer);
        return Subscription<T>(m_state, &observer);
    }

    void RegisterObserver(ObserverType & observer) override
    {
        if (!m_activeObservers.contains(&observer))
        {
            m_observers.push_back(&observer);
            m_activeObservers.insert(&observer);
        }
    }

    void NotifyObservers() override
    {
        T data = GetChangedData();
        
        std::vector<ObserverType*> snapshot = m_observers;
        
        std::set<ObserverType*> notifiedObservers;
        
        for (auto* observer : snapshot)
        {
            if (m_activeObservers.contains(observer) && !notifiedObservers.contains(observer))
            {
                notifiedObservers.insert(observer);
                observer->Update(data);
            }
        }
        m_observers.erase(
            std::remove_if(m_observers.begin(), m_observers.end(),
                [this](ObserverType* obs) {
                    return !m_activeObservers.contains(obs);
                }),
            m_observers.end()
        );
    }

    void RemoveObserver(ObserverType& observer) override
    {
        m_activeObservers.erase(&observer);
    }

    bool IsObserverRegistered(IObserver<T>& observer) const {
         return m_activeObservers.contains(&observer);
    }

protected:
    virtual T GetChangedData() const = 0;

private:
    std::vector<ObserverType*> m_observers;
    std::set<ObserverType*> m_activeObservers;
    std::shared_ptr<SubscriptionState<T>> m_state;
};
