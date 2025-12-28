#ifndef INDICATORS_H
#define INDICATORS_H

#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <tuple>
#include <limits>

// Pure C++ Indicator Functions
std::vector<double> compute_sma(const std::vector<double>& input, const int& window);

std::vector<double> compute_ema(const std::vector<double>& input, const int& window);

std::vector<double> compute_rsi(const std::vector<double>& input, const int& window);

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> 
compute_bollinger_bands(const std::vector<double>& input, const int& window, const double& k);

std::tuple<std::vector<double>, std::vector<double>> 
compute_macd(const std::vector<double>& input, const int& fast_p, const int& slow_p, const int& signal_p);

// Logic and Strategy
int compute_signals(const std::vector<double>& rsi, 
                    const std::vector<double>& price,
                    const std::vector<double>& bb_upper, 
                    const std::vector<double>& bb_lower);

#endif