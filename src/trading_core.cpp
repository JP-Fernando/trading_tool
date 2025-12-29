#include "trading_core.h"
#include "market_manager.h"

/// @brief Calculates the Simple Moving Average (SMA) of the input data over a specified window.
py::array_t<double> calculate_sma_cpp(const py::array_t<double> &input_data, const int &window)
{
    // 1. Convert input data to std::vector
    auto buf = input_data.request();
    std::vector<double> input_vec(static_cast<double *>(buf.ptr),
                                  static_cast<double *>(buf.ptr) + buf.size);

    // 2. Call function
    std::vector<double> res_vec = compute_sma(input_vec, window);

    // 3. Convert result to py::array_t
    return py::cast(res_vec);
}

/// @brief Calculates the Exponential Moving Average (EMA) of the input data over a specified window.
py::array_t<double> calculate_ema_cpp(const py::array_t<double> &input_data, const int &window)
{
    // 1. Convert input data to std::vector
    auto buf = input_data.request();
    std::vector<double> input_vec(static_cast<double *>(buf.ptr),
                                  static_cast<double *>(buf.ptr) + buf.size);

    // 2. Call function
    std::vector<double> res_vec = compute_ema(input_vec, window);

    // 3. Convert result to py::array_t
    return py::cast(res_vec);
}

/// @brief Calculates the Relative Strength Index (RSI) of the input data over a specified window.
py::array_t<double> calculate_rsi_cpp(const py::array_t<double> &input_data, const int &window)
{
    // 1. Convert input_data to std::vector
    auto buf = input_data.request();
    std::vector<double> input_vec(static_cast<double *>(buf.ptr),
                                  static_cast<double *>(buf.ptr) + buf.size);

    // 2. Call function
    std::vector<double> result_vec = compute_rsi(input_vec, window);

    // 3. Convert result to py::array_t
    return py::cast(result_vec);
}

/// @brief Calculates the Bollinger Bands of the input data over a specified window.
std::tuple<py::array_t<double>, py::array_t<double>, py::array_t<double>>
calculate_bollinger_bands_cpp(const py::array_t<double> &input_data, const int &window, const double &k)
{
    // 1. Convert input data to std::vector
    auto buf_in = input_data.request();
    std::vector<double> input_vec(static_cast<double *>(buf_in.ptr),
                                  static_cast<double *>(buf_in.ptr) + buf_in.size);

    // 2. Call function
    auto [upper_v, mid_v, lower_v] = compute_bollinger_bands(input_vec, window, k);

    // 3. Convert to tuple of py::array_t
    return std::make_tuple(
        py::cast(upper_v),
        py::cast(mid_v),
        py::cast(lower_v));
}

/// @brief Calculates the Moving Average Convergence Divergence (MACD) of the input data.
std::tuple<py::array_t<double>, py::array_t<double>>
calculate_macd_cpp(const py::array_t<double> &input_data,
                   const int &fast,
                   const int &slow,
                   const int &signal)
{
    // 1. Convert input data to std::vector
    auto buf = input_data.request();
    std::vector<double> input_vec(static_cast<double *>(buf.ptr),
                                  static_cast<double *>(buf.ptr) + buf.size);

    // 2. Call function
    auto [macd_vec, signal_vec] = compute_macd(input_vec, fast, slow, signal);

    // 3. Convert to tuple of py::array_t
    return std::make_tuple(
        py::cast(macd_vec),
        py::cast(signal_vec));
}

/// @brief Checks trading signals based on RSI and Bollinger Bands.
int check_signals_cpp(const py::array_t<double> &rsi,
                      const py::array_t<double> &price,
                      const py::array_t<double> &bb_upper,
                      const py::array_t<double> &bb_lower)
{

    auto r = rsi.unchecked<1>();
    auto p = price.unchecked<1>();
    auto up = bb_upper.unchecked<1>();
    auto lo = bb_lower.unchecked<1>();

    size_t last = r.shape(0) - 1;

    // Buy if price below lower band
    if (r(last) < 30 && p(last) < lo(last))
    {
        return 1; // BUY
    }
    // Sell if price above upper band
    else if (r(last) > 70 && p(last) > up(last))
    {
        return -1; // SELL
    }

    return 0; // HOLD
}

/// @brief Macro of Pybind11 which defines module `trading_core`.
/// This module exposes high-performance indicator functions
/// and the MarketManager class for multithreaded processing.
PYBIND11_MODULE(trading_core, m)
{
    m.doc() = "Multithreaded trading core engine for real-time market data processing.";

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
             "    threads (int): Number of background worker threads.",
             py::arg("threads") = 4)
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