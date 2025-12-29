#include "core/indicators.h"

/// @brief Calculates the Simple Moving Average (SMA) of the input data over a specified window.
std::vector<double> compute_sma(const std::vector<double>& input, const int& window) {
    const size_t size = input.size();
    constexpr double nan = std::numeric_limits<double>::quiet_NaN();
    std::vector<double> result(size, nan);

    if (size < (size_t)window) return result;

    // Initial window sum
    double current_sum = std::accumulate(input.begin(), input.begin() + window, 0.0);
    result[window - 1] = current_sum / window;

    // Sliding window logic O(n)
    for (size_t i = window; i < size; ++i) {
        current_sum += input[i] - input[i - window];
        result[i] = current_sum / window;
    }

    return result;
}

/// @brief Calculates the Exponential Moving Average (EMA) of the input data over a specified window.
std::vector<double> compute_ema(const std::vector<double>& input, const int& window) {
    const size_t size = input.size();
    std::vector<double> result(size);

    if (size == 0) return result;

    const double alpha = 2.0 / (window + 1.0);
    const double beta = 1.0 - alpha;
    
    result[0] = input[0];

    for (size_t i = 1; i < size; ++i) {
        result[i] = (input[i] * alpha) + (result[i - 1] * beta);
    }

    return result;
}

/// @brief Calculates the Relative Strength Index (RSI) of the input data over a specified window.
std::vector<double> compute_rsi(const std::vector<double>& input, const int& window) {
    size_t size = input.size();
    std::vector<double> rsi(size, std::numeric_limits<double>::quiet_NaN());

    if (size <= (size_t)window) return rsi;

    double avg_gain = 0.0, avg_loss = 0.0;
    double alpha = 1.0 / window;

    for (size_t i = 1; i <= (size_t)window; ++i) {
        double diff = input[i] - input[i - 1];
        if (diff >= 0) avg_gain += diff;
        else avg_loss -= diff;
    }
    avg_gain /= window; avg_loss /= window;

    auto calc = [](double g, double l) { 
        return (l == 0) ? 100.0 : 100.0 - (100.0 / (1.0 + g / l)); 
    };

    rsi[window] = calc(avg_gain, avg_loss);

    for (size_t i = window + 1; i < size; ++i) {
        double diff = input[i] - input[i - 1];
        double gain = std::max(0.0, diff);
        double loss = std::max(0.0, -diff);
        avg_gain = (gain * alpha) + (avg_gain * (1.0 - alpha));
        avg_loss = (loss * alpha) + (avg_loss * (1.0 - alpha));
        rsi[i] = calc(avg_gain, avg_loss);
    }
    return rsi;
}


/// @brief Calculates the Bollinger Bands of the input data over a specified window.
std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> 
compute_bollinger_bands(const std::vector<double>& input, const int& window, const double& k) {
    const ssize_t size = input.size();

    constexpr double nan = std::numeric_limits<double>::quiet_NaN();
    auto upper_arr = std::vector<double>(size, nan);
    auto mid_arr = std::vector<double>(size, nan);
    auto lower_arr = std::vector<double>(size, nan);

    if (size < window) {
        return std::make_tuple(upper_arr, mid_arr, lower_arr);
    }

    double sum = 0.0;
    double sum_sq = 0.0;

    // Initial window
    for (ssize_t i = 0; i < window; ++i) {
        sum += input[i];
        sum_sq += input[i] * input[i];
    }

    auto compute_bands = [&](ssize_t idx, double current_sum, double current_sum_sq) {
        double mean = current_sum / window;
        // Variance formula: (sum_sq - (sum^2 / N)) / N
        double variance = (current_sum_sq - (current_sum * current_sum / window)) / window;
        double std_dev = std::sqrt(std::max(0.0, variance)); // std::max handles precision noise

        mid_arr[idx] = mean;
        upper_arr[idx] = mean + (k * std_dev);
        lower_arr[idx] = mean - (k * std_dev);
    };

    compute_bands(window - 1, sum, sum_sq);

    // Sliding window logic
    for (ssize_t i = window; i < size; ++i) {
        sum += input[i] - input[i - window];
        sum_sq += (input[i] * input[i]) - (input[i - window] * input[i - window]);
        compute_bands(i, sum, sum_sq);
    }

    return std::make_tuple(upper_arr, mid_arr, lower_arr);
}

/// @brief Calculates the Moving Average Convergence Divergence (MACD) of the input data.
std::tuple<std::vector<double>, std::vector<double>> 
compute_macd(const std::vector<double>& input, 
             const int& fast_period, 
             const int& slow_period, 
             const int& signal_period) {
    
    const size_t size = input.size();
    constexpr double nan = std::numeric_limits<double>::quiet_NaN();
    
    std::vector<double> macd_line(size, nan);
    std::vector<double> signal_line(size, nan);

    if (size == 0) return {macd_line, signal_line};

    // EMA Constants
    const double alpha_fast = 2.0 / (fast_period + 1.0);
    const double alpha_slow = 2.0 / (slow_period + 1.0);
    const double alpha_sig  = 2.0 / (signal_period + 1.0);

    double current_ema_fast = input[0];
    double current_ema_slow = input[0];
    
    macd_line[0] = current_ema_fast - current_ema_slow;

    // Step 1: Calculate MACD Line (Difference of two EMAs)
    for (size_t i = 1; i < size; ++i) {
        current_ema_fast = (input[i] * alpha_fast) + (current_ema_fast * (1.0 - alpha_fast));
        current_ema_slow = (input[i] * alpha_slow) + (current_ema_slow * (1.0 - alpha_slow));
        macd_line[i] = current_ema_fast - current_ema_slow;
    }

    // Step 2: Calculate Signal Line (EMA of the MACD Line)
    signal_line[0] = macd_line[0];
    for (size_t i = 1; i < size; ++i) {
        // We only compute signal if macd_line at that point is not NaN
        if (!std::isnan(macd_line[i])) {
            signal_line[i] = (macd_line[i] * alpha_sig) + (signal_line[i - 1] * (1.0 - alpha_sig));
        }
    }

    return {macd_line, signal_line};
}

// Logic and Strategy
/// @brief Checks trading signals based on RSI and Bollinger Bands.
int compute_signals(const std::vector<double>& rsi, const std::vector<double>& price,
                    const std::vector<double>& bb_upper, const std::vector<double>& bb_lower) {
    if (rsi.empty() || std::isnan(rsi.back())) return 0;
    
    size_t last = rsi.size() - 1;
    if (rsi[last] < 30.0 && price[last] < bb_lower[last]) return 1;  // BUY
    if (rsi[last] > 70.0 && price[last] > bb_upper[last]) return -1; // SELL
    return 0;
}