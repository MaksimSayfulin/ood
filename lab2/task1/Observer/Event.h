#pragma once

#include <vector>
#include <set>
#include <functional>
#include <algorithm>
#include <memory>
#include <atomic>

template <typename... Args>
class Event;

template <typename... Args>
struct EventState {
    std::atomic<bool> alive{true};
    std::set<size_t> activeHandlers;
    std::set<size_t> pendingActive;
};

template <typename... Args>
class EventSubscription {
public:
    EventSubscription() = default;

    ~EventSubscription() {
        Disconnect();
    }

    void Disconnect() {
        if (m_state && m_state->alive) {
            m_state->activeHandlers.erase(m_handlerId);
            m_state->pendingActive.erase(m_handlerId);
        }
        m_state.reset();
        m_handlerId = 0;
    }

    EventSubscription(EventSubscription&& other) noexcept
        : m_state(std::move(other.m_state)), m_handlerId(other.m_handlerId) {
        other.m_state.reset();
        other.m_handlerId = 0;
    }

    EventSubscription& operator=(EventSubscription&& other) noexcept {
        if (this != &other) {
            Disconnect();
            m_state = std::move(other.m_state);
            m_handlerId = other.m_handlerId;
            other.m_state.reset();
            other.m_handlerId = 0;
        }
        return *this;
    }

    EventSubscription(const EventSubscription&) = delete;
    EventSubscription& operator=(const EventSubscription&) = delete;

private:
    friend class Event<Args...>;

    EventSubscription(std::shared_ptr<EventState<Args...>> state, size_t handlerId)
        : m_state(std::move(state)), m_handlerId(handlerId) {}

    std::shared_ptr<EventState<Args...>> m_state;
    size_t m_handlerId = 0;
};

template <typename... Args>
class Event {
public:
    using Handler = std::function<void(Args...)>;

    Event() : m_state(std::make_shared<EventState<Args...>>()) {
        m_state->alive = true;
    }

    ~Event() {
        if (m_state) {
            m_state->alive = false;
        }
    }

    Event(Event&& other) noexcept
        : m_handlers(std::move(other.m_handlers))
        , m_pendingHandlers(std::move(other.m_pendingHandlers))
        , m_state(std::move(other.m_state))
        , m_nextHandlerId(other.m_nextHandlerId)
        , m_isNotifying(false)
    {
    }

    Event& operator=(Event&& other) noexcept {
        if (this != &other) {
            m_handlers = std::move(other.m_handlers);
            m_pendingHandlers = std::move(other.m_pendingHandlers);
            m_state = std::move(other.m_state);
            m_nextHandlerId = other.m_nextHandlerId;
            m_isNotifying = false;
        }
        return *this;
    }

    EventSubscription<Args...> Subscribe(Handler handler) {
        size_t handlerId = m_nextHandlerId++;
        bool alreadyNotifying = m_isNotifying;
        if (!alreadyNotifying) {
            m_handlers.push_back({handlerId, std::move(handler)});
            m_state->activeHandlers.insert(handlerId);
        } else {
            m_pendingHandlers.push_back({handlerId, std::move(handler)});
            m_state->pendingActive.insert(handlerId);
        }
        return EventSubscription<Args...>(m_state, handlerId);
    }

    void Notify(Args... args) {
        if (!m_pendingHandlers.empty()) {
            for (auto& entry : m_pendingHandlers) {
                m_handlers.push_back({entry.id, std::move(entry.handler)});
            }
            for (auto id : m_state->pendingActive) {
                m_state->activeHandlers.insert(id);
            }
            m_pendingHandlers.clear();
            m_state->pendingActive.clear();
        }

        m_isNotifying = true;

        auto snapshot = m_handlers;
        std::set<size_t> notifiedHandlers;

        for (auto& entry : snapshot) {
            if (m_state->activeHandlers.find(entry.id) != m_state->activeHandlers.end() && !notifiedHandlers.contains(entry.id))
            {
                notifiedHandlers.insert(entry.id);
                entry.handler(args...);
            }
        }

        m_isNotifying = false;

        m_handlers.erase(
            std::remove_if(m_handlers.begin(), m_handlers.end(),
                [this](const HandlerEntry& entry) {
                    return m_state->activeHandlers.find(entry.id) == m_state->activeHandlers.end();
                }),
            m_handlers.end()
        );
    }

private:
    struct HandlerEntry {
        size_t id;
        Handler handler;
    };

    std::vector<HandlerEntry> m_handlers;
    std::vector<HandlerEntry> m_pendingHandlers;
    std::shared_ptr<EventState<Args...>> m_state;
    size_t m_nextHandlerId = 1;
    bool m_isNotifying = false;
};
