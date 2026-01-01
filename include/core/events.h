#ifndef EVENTS_H
#define EVENTS_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>

namespace trading::events {

using Timestamp = std::chrono::nanoseconds;
using Price = double;
using Volume = double;
using OrderId = uint64_t;

// ============================================================================
// Event Types
// ============================================================================

enum class EventType : uint8_t {
    TICK,
    SIGNAL,
    ORDER,
    FILL,
    POSITION_UPDATE,
    PNL_UPDATE
};

enum class Side : uint8_t {
    BUY,
    SELL
};

enum class OrderStatus : uint8_t {
    PENDING,
    SUBMITTED,
    FILLED,
    PARTIALLY_FILLED,
    CANCELLED,
    REJECTED
};

// ============================================================================
// Market Tick Event
// ============================================================================

struct TickEvent {
    Timestamp timestamp;
    std::string symbol;
    Price bid;
    Price ask;
    Volume bid_volume;
    Volume ask_volume;
    Price last;  // Last traded price
    Volume last_volume;

    EventType type() const noexcept { return EventType::TICK; }
};

// ============================================================================
// Signal Event (strategy-generated)
// ============================================================================

struct SignalEvent {
    Timestamp timestamp;
    std::string symbol;
    Side side;
    double strength;  // Signal strength [-1.0, 1.0]
    std::string strategy_id;

    EventType type() const noexcept { return EventType::SIGNAL; }
};

// ============================================================================
// Order Event (trading order)
// ============================================================================

struct OrderEvent {
    OrderId order_id;
    Timestamp timestamp;
    std::string symbol;
    Side side;
    Volume quantity;
    Price limit_price;  // 0.0 for market orders
    OrderStatus status;
    std::string strategy_id;

    EventType type() const noexcept { return EventType::ORDER; }

    bool is_market_order() const noexcept {
        return limit_price == 0.0;
    }
};

// ============================================================================
// Fill Event (execute order)
// ============================================================================

struct FillEvent {
    OrderId order_id;
    Timestamp timestamp;
    std::string symbol;
    Side side;
    Volume filled_quantity;
    Price fill_price;
    double commission;  // Exchange commissions
    double slippage;    // Applied slippage
    std::string exchange;

    EventType type() const noexcept { return EventType::FILL; }
};

// ============================================================================
// Position Update Event
// ============================================================================

struct PositionUpdateEvent {
    Timestamp timestamp;
    std::string symbol;
    Volume position;  // Net position (positive = long, negative = short)
    Price avg_entry_price;
    double unrealized_pnl;
    double realized_pnl;

    EventType type() const noexcept { return EventType::POSITION_UPDATE; }
};

// ============================================================================
// PnL Update Event
// ============================================================================

struct PnLUpdateEvent {
    Timestamp timestamp;
    double total_pnl;
    double realized_pnl;
    double unrealized_pnl;
    double commission_paid;
    size_t total_trades;
    size_t winning_trades;

    EventType type() const noexcept { return EventType::PNL_UPDATE; }

    double win_rate() const noexcept {
        return total_trades > 0
            ? static_cast<double>(winning_trades)
                  / static_cast<double>(total_trades)
            : 0.0;
    }
};

// ============================================================================
// Event Variant (zero virtual calls: std::variant to avoid virtual dispatch)
// ============================================================================

using Event = std::variant<
    TickEvent,
    SignalEvent,
    OrderEvent,
    FillEvent,
    PositionUpdateEvent,
    PnLUpdateEvent>;

// ============================================================================
// Event Utilities
// ============================================================================

inline Timestamp get_timestamp(const Event& event) {
    return std::visit([](const auto& e) { return e.timestamp; }, event);
}

inline EventType get_event_type(const Event& event) {
    return std::visit([](const auto& e) { return e.type(); }, event);
}

struct EventComparator {
    bool operator()(const Event& a, const Event& b) const {
        return get_timestamp(a) > get_timestamp(b);
    }
};

inline Timestamp make_timestamp(int64_t nanos_since_epoch) {
    return Timestamp{nanos_since_epoch};
}

inline Timestamp now() {
    return std::chrono::duration_cast<Timestamp>(
        std::chrono::system_clock::now().time_since_epoch());
}

}  // namespace trading::events

#endif
