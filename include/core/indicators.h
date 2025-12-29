#ifndef INDICATORS_H
#define INDICATORS_H

#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <tuple>
#include <limits>

// Pure C++ Indicator Functions

/// @brief Calculates the Simple Moving Average (SMA) of the input data over a specified window.
/// @param input Input data as a vector of doubles.
/// @param window Window size for the SMA calculation.
/// @return SMA values as a vector of doubles.
std::vector<double> compute_sma(const std::vector<double>& input, const int& window);

/// @brief Calculates the Exponential Moving Average (EMA) of the input data over a specified window.
/// @param input Input data as a vector of doubles.
/// @param window Window size for the EMA calculation.
/// @return EMA values as a vector of doubles.
std::vector<double> compute_ema(const std::vector<double>& input, const int& window);

/// @brief Calculates the Relative Strength Index (RSI) of the input data over a specified window.
/// @param input Input data as a vector of doubles.
/// @param window Window size for the RSI calculation.
/// @return RSI values as a vector of doubles.
std::vector<double> compute_rsi(const std::vector<double>& input, const int& window);

/// @brief Calculates the Bollinger Bands of the input data over a specified window.
/// @param input Input data as a vector of doubles.
/// @param window Window size for the Bollinger Bands calculation.
/// @param k Standard deviation multiplier.
/// @return Tuple of (upper_band, middle_band, lower_band) as vectors of doubles.
std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> 
compute_bollinger_bands(const std::vector<double>& input, const int& window, const double& k);

/// @brief Calculates the Moving Average Convergence Divergence (MACD) of the input data.
/// @param input Input data as a vector of doubles.
/// @param fast_period Fast EMA window size.
/// @param slow_period Slow EMA window size.
/// @param signal_period Signal line EMA window size.
/// @return Tuple of (macd_line, signal_line) as vectors of doubles.
std::tuple<std::vector<double>, std::vector<double>> 
compute_macd(const std::vector<double>& input, const int& fast_period, const int& slow_period, const int& signal_period);

// Logic and Strategy
/// @brief Checks trading signals based on RSI and Bollinger Bands.
/// @param rsi RSI values as a vector of doubles.
/// @param price Price values as a vector of doubles.
/// @param bb_upper Upper Bollinger Band values as a vector of doubles.
/// @param bb_lower Lower Bollinger Band values as a vector of doubles.
/// @return Trading signal: 1 for buy, -1 for sell, 0 for hold.
int compute_signals(const std::vector<double>& rsi, 
                    const std::vector<double>& price,
                    const std::vector<double>& bb_upper, 
                    const std::vector<double>& bb_lower);

#endif