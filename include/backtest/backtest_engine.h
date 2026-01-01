#ifndef BACKTEST_ENGINE_H
#define BACKTEST_ENGINE_H

#include <atomic>
#include <cstddef>
#include <memory>

#include "backtest/event_queue.h"
#include "backtest/execution_engine.h"
#include "core/events.h"

namespace trading::strategies { class StrategyBase; }
namespace trading::risk { class RiskManager; }
namespace trading::core { class PortfolioManager; }

namespace trading::backtest {

class BacktestEngine {
public:
    BacktestEngine(
        std::shared_ptr<EventQueue> queue,
        std::shared_ptr<ExecutionEngine> execution);

    void run();
    void stop();

    void registerFillEvent(const FillEvent& fill);
    std::shared_ptr<EventQueue> getQueue() const;

    void push_event(const Event& event);

private:
    void handle_event(const Event& event);

    void handle_tick(const TickEvent& tick);
    void handle_signal(const SignalEvent& signal);
    void handle_order(const OrderEvent& order);
    void handle_fill(const FillEvent& fill);

    std::shared_ptr<EventQueue> queue_;
    std::shared_ptr<ExecutionEngine> execution_engine_;

    std::atomic<bool> running_{false};
    size_t events_processed_{0};
};

} // namespace trading::backtest

#endif // BACKTEST_ENGINE_H