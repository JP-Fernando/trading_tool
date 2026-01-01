#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "core/events.h"

namespace trading::backtest {

using trading::events::Event;
using trading::events::EventComparator;

// ============================================================================
// Thread-safe Priority Event Queue (Chronological Processing)
// ============================================================================

class EventQueue {
public:
    EventQueue() = default;
    ~EventQueue() = default;

    EventQueue(const EventQueue&) = delete;
    EventQueue& operator=(const EventQueue&) = delete;

    void push(Event event) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(event));
        }
        cv_.notify_one();
    }

    Event pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]() { return !queue_.empty() || stopped_; });

        if (queue_.empty()) {
            return {};
        }

        Event event = std::move(queue_.top());
        queue_.pop();
        return event;
    }

    bool try_pop(Event& out_event) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        out_event = std::move(queue_.top());
        queue_.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
    }

    bool stopped() const noexcept {
        return stopped_;
    }

private:
    std::priority_queue<Event, std::vector<Event>, EventComparator> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stopped_{false};
};

} // namespace trading::backtest

#endif // EVENT_QUEUE_H