#include "backtest/backtest_engine.h"

#include <string>
#include <type_traits>
#include <utility>

#include "utils/logger.h"

namespace trading::backtest {

BacktestEngine::BacktestEngine(
    std::shared_ptr<EventQueue> queue,
    std::shared_ptr<ExecutionEngine> execution)
    : queue_(std::move(queue)),
      execution_engine_(std::move(execution)) {}

void BacktestEngine::run() {
    running_ = true;
    Logger::log(LogLevel::INFO, "Starting Backtest Engine Loop...");

    while (running_ && !queue_->empty()) {
        Event event = queue_->pop();
        handle_event(event);
        events_processed_++;
    }

    Logger::log(
        LogLevel::INFO,
        "Backtest finished. Processed " + std::to_string(events_processed_)
            + " events.");
}

void BacktestEngine::stop() {
    running_ = false;
    queue_->stop();
}

void BacktestEngine::push_event(const Event& event) {
    queue_->push(event);
}

void BacktestEngine::handle_event(const Event& event) {
    std::visit(
        [this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, TickEvent>) {
                this->handle_tick(arg);
            } else if constexpr (std::is_same_v<T, SignalEvent>) {
                this->handle_signal(arg);
            } else if constexpr (std::is_same_v<T, OrderEvent>) {
                this->handle_order(arg);
            } else if constexpr (std::is_same_v<T, FillEvent>) {
                this->handle_fill(arg);
            }
        },
        event);
}

void BacktestEngine::handle_tick(const TickEvent& tick) {
    execution_engine_->on_tick(tick);
}

void BacktestEngine::handle_signal(const SignalEvent& signal) {
    Logger::log(LogLevel::INFO, "Signal received: " + signal.symbol);

    OrderEvent order{
        .order_id = static_cast<uint64_t>(events_processed_),
        .timestamp = signal.timestamp,
        .symbol = signal.symbol,
        .side = signal.side,
        .quantity = 1.0,
        .limit_price = 0.0,
        .status = OrderStatus::PENDING,
        .strategy_id = signal.strategy_id};

    queue_->push(order);
}

void BacktestEngine::handle_order(const OrderEvent& order) {
    execution_engine_->on_order(order);
}

void BacktestEngine::handle_fill(const FillEvent& fill) {
    std::string side = (fill.side == Side::BUY) ? "BUY" : "SELL";
    std::string msg = "[FILL] " + fill.symbol + " " + side + " @ "
        + std::to_string(fill.fill_price);
    Logger::log(LogLevel::SIGNAL, msg);
}

void BacktestEngine::registerFillEvent(const FillEvent& fill) {
    execution_engine_->fills_history.push_back(fill);
}

std::shared_ptr<EventQueue> BacktestEngine::getQueue() const {
    return queue_;
}

} // namespace trading::backtest
