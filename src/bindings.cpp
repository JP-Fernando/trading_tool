#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h> 
#include <string>

#include "core/market_manager.h"
#include "core/indicators.h"
#include "core/trading_core.h"
#include "utils/logger.h"

namespace py = pybind11;      

/// @brief Macro of Pybind11 which defines module `trading_core`.
/// This module exposes high-performance indicator functions
/// and the MarketManager class for multithreaded processing.
PYBIND11_MODULE(trading_core, m)
{
    m.doc() = "Multithreaded trading core engine for real-time market data processing.";

    // --- Logging ---

    py::enum_<LogLevel>(m, "LogLevel")
        .value("INFO", LogLevel::INFO)
        .value("WARNING", LogLevel::WARNING)
        .value("ERROR", LogLevel::ERROR)
        .value("SIGNAL", LogLevel::SIGNAL)
        .export_values();

    m.def("set_log_callback", [](LogCallback cb) {
        Logger::set_callback(cb);
    }, "Configure Python function to get logs C++ logs.",
    py::arg("callback"));

    // --- Indicators ---

    m.def("calculate_sma", &calculate_sma_cpp,
          "Calculates the Simple Moving Average (SMA).\n\n"
          "Args:\n"
          "    input_data (np.ndarray): Array of price data.\n"
          "    window (int): Size of the sliding window.\n\n"
          "Returns:\n"
          "    np.ndarray: The SMA series.",
          py::arg("input_data"), py::arg("window"));

    m.def("calculate_ema", &calculate_ema_cpp,
          "Calculates the Exponential Moving Average (EMA).\n\n"
          "Args:\n"
          "    input_data (np.ndarray): Array of price data.\n"
          "    window (int): The period for the exponential weight.\n\n"
          "Returns:\n"
          "    np.ndarray: The EMA series.",
          py::arg("input_data"), py::arg("window"));

    m.def("calculate_rsi", &calculate_rsi_cpp,
          "Calculates the Relative Strength Index (RSI) using Wilder's smoothing.\n\n"
          "Args:\n"
          "    input_data (np.ndarray): Array of price data.\n"
          "    window (int): Lookback period (commonly 14).\n\n"
          "Returns:\n"
          "    np.ndarray: RSI values ranging from 0 to 100.",
          py::arg("input_data"), py::arg("window") = 14);

    m.def("calculate_macd", &calculate_macd_cpp,
          "Calculates MACD Line and Signal Line.\n\n"
          "Args:\n"
          "    input_data (np.ndarray): Price series.\n"
          "    fast (int): Fast EMA window (default 12).\n"
          "    slow (int): Slow EMA window (default 26).\n"
          "    signal (int): Signal line EMA window (default 9).\n\n"
          "Returns:\n"
          "    tuple: (macd_line, signal_line) as numpy arrays.",
          py::arg("input_data"), py::arg("fast") = 12, py::arg("slow") = 26, py::arg("signal") = 9);

    m.def("calculate_bollinger_bands", &calculate_bollinger_bands_cpp,
          "Calculates Bollinger Bands (Upper, Middle, Lower).\n\n"
          "Args:\n"
          "    input_data (np.ndarray): Price series.\n"
          "    window (int): Standard deviation lookback (default 20).\n"
          "    k (float): Standard deviation multiplier (default 2.0).\n\n"
          "Returns:\n"
          "    tuple: (upper_band, middle_band, lower_band).",
          py::arg("input_data"), py::arg("window") = 20, py::arg("k") = 2.0);

    m.def("check_signals", &check_signals_cpp,
          "Detects basic oversold/overbought signals based on RSI and Bollinger Bands.\n\n"
          "Returns:\n"
          "    int: 1 (Buy), -1 (Sell), 0 (Hold).",
          py::arg("rsi"), py::arg("price"), py::arg("bb_upper"), py::arg("bb_lower"));

    // --- Market Manager ---

    py::class_<MarketManager>(m, "MarketManager", "Orchestrator for parallel market data processing.")
        .def(py::init<size_t>(),
             "Initializes the manager with a thread pool.\n\n"
             "Args:\n"
             "    num_threads (int): Number of background worker threads.",
             py::arg("num_threads") = 4)
        .def("update_tick", &MarketManager::update_tick,
             "Dispatches a new price tick to the thread pool for analysis.\n\n"
             "Args:\n"
             "    symbol (str): The ticker symbol (e.g., 'BTC/USDT').\n"
             "    price (float): The current market price.",
             py::arg("symbol"), py::arg("price"))
        .def("get_last_price", &MarketManager::get_last_price,
             "Thread-safe retrieval of the last stored price for a symbol.",
             py::arg("symbol"));
}