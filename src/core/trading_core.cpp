#include "core/trading_core.h"
#include "core/market_manager.h"

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
