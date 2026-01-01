#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>
#include <string>
#include <utility>

#include "backtest/backtest_engine.h"
#include "backtest/event_queue.h"
#include "backtest/execution_engine.h"
#include "core/events.h"
#include "core/indicators.h"
#include "core/market_manager.h"
#include "core/trading_core.h"
#include "utils/logger.h"

namespace py = pybind11;
using namespace trading::events;
using namespace trading::backtest;

namespace {
int64_t timestamp_to_nanos(const Timestamp& ts) {
    return ts.count();
}

Timestamp nanos_to_timestamp(int64_t nanos) {
    return make_timestamp(nanos);
}
} // namespace

/// @brief Macro of Pybind11 which defines module `trading_core`.
/// This module exposes high-performance indicator functions
/// and the MarketManager class for multithreaded processing.
PYBIND11_MODULE(trading_core, m) {
    m.doc() = "Multithreaded trading core engine for real-time market data processing.";

    // --- Logging ---

    py::enum_<LogLevel>(m, "LogLevel")
        .value("INFO", LogLevel::INFO)
        .value("WARNING", LogLevel::WARNING)
        .value("ERROR", LogLevel::ERROR)
        .value("SIGNAL", LogLevel::SIGNAL)
        .export_values();

    m.def("set_log_callback",
          [](LogCallback cb) { Logger::set_callback(cb); },
          "Configure Python function to get logs C++ logs.",
          py::arg("callback"));

    // --- Indicators ---

    m.def("calculate_sma",
          &calculate_sma_cpp,
          "Calculates the Simple Moving Average (SMA).\n\n"
          "Args:\n"
          "    input_data (np.ndarray): Array of price data.\n"
          "    window (int): Size of the sliding window.\n\n"
          "Returns:\n"
          "    np.ndarray: The SMA series.",
          py::arg("input_data"),
          py::arg("window"));

    m.def("calculate_ema",
          &calculate_ema_cpp,
          "Calculates the Exponential Moving Average (EMA).\n\n"
          "Args:\n"
          "    input_data (np.ndarray): Array of price data.\n"
          "    window (int): The period for the exponential weight.\n\n"
          "Returns:\n"
          "    np.ndarray: The EMA series.",
          py::arg("input_data"),
          py::arg("window"));

    m.def("calculate_rsi",
          &calculate_rsi_cpp,
          "Calculates the Relative Strength Index (RSI) using Wilder's smoothing.\n\n"
          "Args:\n"
          "    input_data (np.ndarray): Array of price data.\n"
          "    window (int): Lookback period (commonly 14).\n\n"
          "Returns:\n"
          "    np.ndarray: RSI values ranging from 0 to 100.",
          py::arg("input_data"),
          py::arg("window") = 14);

    m.def("calculate_macd",
          &calculate_macd_cpp,
          "Calculates MACD Line and Signal Line.\n\n"
          "Args:\n"
          "    input_data (np.ndarray): Price series.\n"
          "    fast (int): Fast EMA window (default 12).\n"
          "    slow (int): Slow EMA window (default 26).\n"
          "    signal (int): Signal line EMA window (default 9).\n\n"
          "Returns:\n"
          "    tuple: (macd_line, signal_line) as numpy arrays.",
          py::arg("input_data"),
          py::arg("fast") = 12,
          py::arg("slow") = 26,
          py::arg("signal") = 9);

    m.def("calculate_bollinger_bands",
          &calculate_bollinger_bands_cpp,
          "Calculates Bollinger Bands (Upper, Middle, Lower).\n\n"
          "Args:\n"
          "    input_data (np.ndarray): Price series.\n"
          "    window (int): Standard deviation lookback (default 20).\n"
          "    k (float): Standard deviation multiplier (default 2.0).\n\n"
          "Returns:\n"
          "    tuple: (upper_band, middle_band, lower_band).",
          py::arg("input_data"),
          py::arg("window") = 20,
          py::arg("k") = 2.0);

    m.def("check_signals",
          &check_signals_cpp,
          "Detects basic oversold/overbought signals based on RSI and Bollinger Bands.\n\n"
          "Returns:\n"
          "    int: 1 (Buy), -1 (Sell), 0 (Hold).",
          py::arg("rsi"),
          py::arg("price"),
          py::arg("bb_upper"),
          py::arg("bb_lower"));

    // --- Market Manager ---

    py::class_<MarketManager>(m, "MarketManager", "Orchestrator for parallel market data processing.")
        .def(py::init<size_t>(),
             "Initializes the manager with a thread pool.\n\n"
             "Args:\n"
             "    num_threads (int): Number of background worker threads.",
             py::arg("num_threads") = 4)
        .def("update_tick",
             &MarketManager::update_tick,
             "Dispatches a new price tick to the thread pool for analysis.\n\n"
             "Args:\n"
             "    symbol (str): The ticker symbol (e.g., 'BTC/USDT').\n"
             "    price (float): The current market price.",
             py::arg("symbol"),
             py::arg("price"))
        .def("get_last_price",
             &MarketManager::get_last_price,
             "Thread-safe retrieval of the last stored price for a symbol.",
             py::arg("symbol"));

    // --- Events ---

    auto m_ev = m.def_submodule("events", "Trading events and types");

    py::enum_<Side>(m_ev, "Side")
        .value("BUY", Side::BUY)
        .value("SELL", Side::SELL)
        .export_values();

    py::enum_<OrderStatus>(m_ev, "OrderStatus")
        .value("PENDING", OrderStatus::PENDING)
        .value("SUBMITTED", OrderStatus::SUBMITTED)
        .value("FILLED", OrderStatus::FILLED)
        .value("PARTIALLY_FILLED", OrderStatus::PARTIALLY_FILLED)
        .value("CANCELLED", OrderStatus::CANCELLED)
        .value("REJECTED", OrderStatus::REJECTED)
        .export_values();

    py::class_<TickEvent>(m_ev, "TickEvent")
        .def(py::init([](int64_t timestamp,
                         std::string symbol,
                         Price bid,
                         Price ask,
                         Volume bid_volume,
                         Volume ask_volume,
                         Price last,
                         Volume last_volume) {
            return TickEvent{
                nanos_to_timestamp(timestamp),
                std::move(symbol),
                bid,
                ask,
                bid_volume,
                ask_volume,
                last,
                last_volume};
        }),
             py::arg("timestamp"),
             py::arg("symbol"),
             py::arg("bid"),
             py::arg("ask"),
             py::arg("bid_volume"),
             py::arg("ask_volume"),
             py::arg("last"),
             py::arg("last_volume"))
        .def_property("timestamp",
                      [](const TickEvent& self) { return timestamp_to_nanos(self.timestamp); },
                      [](TickEvent& self, int64_t value) {
                          self.timestamp = nanos_to_timestamp(value);
                      })
        .def_readwrite("symbol", &TickEvent::symbol)
        .def_readwrite("bid", &TickEvent::bid)
        .def_readwrite("ask", &TickEvent::ask)
        .def_readwrite("bid_volume", &TickEvent::bid_volume)
        .def_readwrite("ask_volume", &TickEvent::ask_volume)
        .def_readwrite("last", &TickEvent::last)
        .def_readwrite("last_volume", &TickEvent::last_volume);

    py::class_<SignalEvent>(m_ev, "SignalEvent")
        .def(py::init([](int64_t timestamp,
                         std::string symbol,
                         Side side,
                         double strength,
                         std::string strategy_id) {
            return SignalEvent{
                nanos_to_timestamp(timestamp),
                std::move(symbol),
                side,
                strength,
                std::move(strategy_id)};
        }),
             py::arg("timestamp"),
             py::arg("symbol"),
             py::arg("side"),
             py::arg("strength"),
             py::arg("strategy_id"))
        .def_property("timestamp",
                      [](const SignalEvent& self) { return timestamp_to_nanos(self.timestamp); },
                      [](SignalEvent& self, int64_t value) {
                          self.timestamp = nanos_to_timestamp(value);
                      })
        .def_readwrite("symbol", &SignalEvent::symbol)
        .def_readwrite("side", &SignalEvent::side)
        .def_readwrite("strength", &SignalEvent::strength)
        .def_readwrite("strategy_id", &SignalEvent::strategy_id);

    py::class_<OrderEvent>(m_ev, "OrderEvent")
        .def(py::init([](OrderId order_id,
                         int64_t timestamp,
                         std::string symbol,
                         Side side,
                         Volume quantity,
                         Price limit_price,
                         OrderStatus status,
                         std::string strategy_id) {
            return OrderEvent{
                order_id,
                nanos_to_timestamp(timestamp),
                std::move(symbol),
                side,
                quantity,
                limit_price,
                status,
                std::move(strategy_id)};
        }),
             py::arg("order_id"),
             py::arg("timestamp"),
             py::arg("symbol"),
             py::arg("side"),
             py::arg("quantity"),
             py::arg("limit_price"),
             py::arg("status"),
             py::arg("strategy_id"))
        .def_property("timestamp",
                      [](const OrderEvent& self) { return timestamp_to_nanos(self.timestamp); },
                      [](OrderEvent& self, int64_t value) {
                          self.timestamp = nanos_to_timestamp(value);
                      })
        .def_readwrite("order_id", &OrderEvent::order_id)
        .def_readwrite("symbol", &OrderEvent::symbol)
        .def_readwrite("side", &OrderEvent::side)
        .def_readwrite("quantity", &OrderEvent::quantity)
        .def_readwrite("limit_price", &OrderEvent::limit_price)
        .def_readwrite("status", &OrderEvent::status)
        .def_readwrite("strategy_id", &OrderEvent::strategy_id);

    py::class_<FillEvent>(m_ev, "FillEvent")
        .def(py::init<>())
        .def_property("timestamp",
                      [](const FillEvent& self) { return timestamp_to_nanos(self.timestamp); },
                      [](FillEvent& self, int64_t value) {
                          self.timestamp = nanos_to_timestamp(value);
                      })
        .def_readwrite("order_id", &FillEvent::order_id)
        .def_readwrite("symbol", &FillEvent::symbol)
        .def_readwrite("side", &FillEvent::side)
        .def_readwrite("filled_quantity", &FillEvent::filled_quantity)
        .def_readwrite("fill_price", &FillEvent::fill_price)
        .def_readwrite("commission", &FillEvent::commission)
        .def_readwrite("slippage", &FillEvent::slippage)
        .def_readwrite("exchange", &FillEvent::exchange);

    m_ev.def("make_timestamp",
             [](int64_t nanos_since_epoch) { return nanos_since_epoch; },
             "Create a timestamp from nanoseconds",
             py::arg("nanos_since_epoch"));

    m_ev.def("now",
             []() { return timestamp_to_nanos(now()); },
             "Return the current timestamp in nanoseconds");

    // ========================================================================
    // Backtest submodule (trading_core.backtest)
    // ========================================================================

    auto m_bt = m.def_submodule("backtest", "Backtesting engine and execution");

    py::class_<SlippageInput>(m_bt, "SlippageInput")
        .def_readonly("mid_price", &SlippageInput::mid_price)
        .def_readonly("order_qty", &SlippageInput::order_qty)
        .def_readonly("available_liquidity", &SlippageInput::available_liquidity)
        .def_readonly("side", &SlippageInput::side);

    py::class_<EventQueue, std::shared_ptr<EventQueue>>(m_bt, "EventQueue")
        .def(py::init<>())
        .def("push", [](EventQueue& self, const TickEvent& event) { self.push(event); })
        .def("push", [](EventQueue& self, const SignalEvent& event) { self.push(event); })
        .def("push", [](EventQueue& self, const OrderEvent& event) { self.push(event); })
        .def("push", [](EventQueue& self, const FillEvent& event) { self.push(event); })
        .def("pop", [](EventQueue& self) {
            Event event = self.pop();
            return std::visit([](auto&& arg) { return py::cast(arg); }, event);
        })
        .def("empty", &EventQueue::empty)
        .def("size", &EventQueue::size)
        .def("stop", &EventQueue::stop);

    py::class_<ExecutionEngine, std::shared_ptr<ExecutionEngine>>(m_bt, "ExecutionEngine")
        .def(py::init<EventQueue&, SlippageModel>(),
             py::arg("event_queue"),
             py::arg("slippage_model"),
             py::keep_alive<1, 2>())
        .def("get_fills", &ExecutionEngine::getFillsHistory);

    py::class_<BacktestEngine, std::shared_ptr<BacktestEngine>>(m_bt, "BacktestEngine")
        .def(py::init<std::shared_ptr<EventQueue>, std::shared_ptr<ExecutionEngine>>(),
             py::arg("event_queue"),
             py::arg("execution_engine"))
        .def("run", &BacktestEngine::run)
        .def("stop", &BacktestEngine::stop)
        .def("push_event", [](BacktestEngine& self, const TickEvent& event) {
            self.push_event(Event{event});
        })
        .def("push_event", [](BacktestEngine& self, const SignalEvent& event) {
            self.push_event(Event{event});
        })
        .def("push_event", [](BacktestEngine& self, const OrderEvent& event) {
            self.push_event(Event{event});
        })
        .def("push_event", [](BacktestEngine& self, const FillEvent& event) {
            self.push_event(Event{event});
        })
        .def("get_queue", &BacktestEngine::getQueue);
}