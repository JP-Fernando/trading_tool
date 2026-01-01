#ifndef EXECUTION_ENGINE_H
#define EXECUTION_ENGINE_H

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "backtest/event_queue.h"
#include "core/events.h"

namespace trading::backtest {

using namespace trading::events;

// ============================================================================
// Slippage Model (Stateless Function Object)
// ============================================================================

struct SlippageInput {
    Price mid_price;
    Volume order_qty;
    Volume available_liquidity;
    Side side;
};

using SlippageModel = std::function<Price(const SlippageInput&)>;

// ============================================================================
// Execution Engine
// ============================================================================

class ExecutionEngine {
public:
    explicit ExecutionEngine(EventQueue& event_queue, SlippageModel slippage_model)
        : event_queue_(event_queue), slippage_model_(std::move(slippage_model)) {}

    void on_tick(const TickEvent& tick) {
        last_ticks_[tick.symbol] = tick;
    }

    void on_order(const OrderEvent& order) {
        auto it = last_ticks_.find(order.symbol);
        if (it == last_ticks_.end()) {
            return;
        }

        const TickEvent& tick = it->second;

        Price execution_price = compute_execution_price(order, tick);
        Volume fill_qty = order.quantity;

        FillEvent fill{
            .order_id = order.order_id,
            .timestamp = tick.timestamp,
            .symbol = order.symbol,
            .side = order.side,
            .filled_quantity = fill_qty,
            .fill_price = execution_price,
            .commission = compute_commission(fill_qty, execution_price),
            .slippage = execution_price - mid_price(tick),
            .exchange = "SIMULATED"};

        fills_history.push_back(fill);
        event_queue_.push(fill);
    }

    std::vector<FillEvent> getFillsHistory() const {
        return fills_history;
    }

    std::vector<FillEvent> fills_history;

private:
    EventQueue& event_queue_;
    SlippageModel slippage_model_;

    std::unordered_map<std::string, TickEvent> last_ticks_;

    static Price mid_price(const TickEvent& tick) noexcept {
        return (tick.bid + tick.ask) * 0.5;
    }

    Price compute_execution_price(const OrderEvent& order, const TickEvent& tick) {
        Price mid = mid_price(tick);

        SlippageInput input{
            .mid_price = mid,
            .order_qty = order.quantity,
            .available_liquidity = tick.bid_volume + tick.ask_volume,
            .side = order.side};

        Price slipped_price = slippage_model_(input);

        if (!order.is_market_order()) {
            if (order.side == Side::BUY && slipped_price > order.limit_price) {
                return order.limit_price;
            }
            if (order.side == Side::SELL && slipped_price < order.limit_price) {
                return order.limit_price;
            }
        }

        return slipped_price;
    }

    static double compute_commission(Volume qty, Price price) noexcept {
        constexpr double fee_rate = 0.0005; // 5 bps default
        return qty * price * fee_rate;
    }
};

} // namespace trading::backtest

#endif // EXECUTION_ENGINE_H
