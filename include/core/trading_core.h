#ifndef TRADING_CORE_H
#define TRADING_CORE_H

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <vector>
#include <limits>
#include <algorithm>
#include <numeric>
#include <tuple>

namespace py = pybind11;

/// @brief Calculates the Simple Moving Average (SMA) of the input data over a specified window.
/// @details The SMA is calculated using a sliding window algorithm with O(n) complexity. 
/// It maintains a running sum to avoid re-summing the entire window at each step.
/// The formula used is: \f$ SMA = \frac{\sum_{i=1}^{n} P_i}{n} \f$.
/// @param input_data Input data as a numpy array.
/// @param window Window size for the SMA calculation.
/// @return SMA values as a numpy array.
py::array_t<double> calculate_sma_cpp(const py::array_t<double> &input_data, const int &window);

/// @brief Calculates the Exponential Moving Average (EMA) of the input data over a specified window.
/// @details This implementation uses a recursive formula that gives more weight to recent prices.
/// The smoothing factor is defined as \f$ \alpha = \frac{2}{window + 1} \f$.
/// Formula: \f$ EMA_t = P_t \cdot \alpha + EMA_{t-1} \cdot (1 - \alpha) \f$.
/// @param input_data Input data as a numpy array.
/// @param window Window size for the EMA calculation.
/// @return EMA values as a numpy array.
py::array_t<double> calculate_ema_cpp(const py::array_t<double> &input_data, const int &window);

/// @brief Calculates the Relative Strength Index (RSI) of the input data over a specified window.
/// @details Uses Wilder's Smoothing Method for gains and losses. 
/// @note The first window of data is used to initialize the averages, meaning the 
/// first valid RSI value appears at index `window`.
/// @param input_data Input data as a numpy array.
/// @param window Window size for the RSI calculation.
/// @return RSI values as a numpy array.
py::array_t<double> calculate_rsi_cpp(const py::array_t<double> &input_data, const int &window);

/// @brief Calculates the Bollinger Bands of the input data over a specified window.
/// @details Consists of a Middle Band (SMA) and two outer bands calculated using 
/// standard deviation. This function uses a single-pass variance algorithm to 
/// maintain O(n) efficiency.
/// Upper Band = \f$ SMA + (k \cdot \sigma) \f$, Lower Band = \f$ SMA - (k \cdot \sigma) \f$.
/// @param input_data Input data as a numpy array.
/// @param window Window size for the Bollinger Bands calculation.
/// @param k Standard deviation multiplier.
/// @return Tuple of (upper_band, middle_band, lower_band) as numpy arrays.
std::tuple<py::array_t<double>, py::array_t<double>, py::array_t<double>>
calculate_bollinger_bands_cpp(const py::array_t<double> &input_data, const int &window, const double &k);

/// @brief Calculates the Moving Average Convergence Divergence (MACD) of the input data.
/// @details The MACD is the difference between a Fast EMA and a Slow EMA. 
/// The Signal line is an EMA of the MACD line itself.
/// @param input_data Input data as a numpy array.
/// @param fast Fast EMA window size.
/// @param slow Slow EMA window size.
/// @param signal Signal line EMA window size.
/// @return Tuple of (macd_line, signal_line) as numpy arrays.
std::tuple<py::array_t<double>, py::array_t<double>>
calculate_macd_cpp(const py::array_t<double> &input_data, const int &fast, const int &slow, const int &signal);

/// @brief Checks trading signals based on RSI and Bollinger Bands.
/// @details This is a trend-reversal strategy signal generator.
/// - Returns **1 (BUY)** if RSI < 30 (oversold) AND price is below the Lower Bollinger Band.
/// - Returns **-1 (SELL)** if RSI > 70 (overbought) AND price is above the Upper Bollinger Band.
/// - Returns **0 (HOLD)** otherwise.
/// @param rsi RSI values as a numpy array.
/// @param price Price values as a numpy array.
/// @param bb_upper Upper Bollinger Band values as a numpy array.
/// @param bb_lower Lower Bollinger Band values as a numpy array.
/// @return Trading signal: 1 for buy, -1 for sell, 0 for hold.
int check_signals_cpp(const py::array_t<double> &rsi,
                      const py::array_t<double> &price,
                      const py::array_t<double> &bb_upper,
                      const py::array_t<double> &bb_lower);

#endif