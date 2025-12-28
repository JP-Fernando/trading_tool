#include "trading_core.h"


py::array_t<double> calculate_sma_cpp(const py::array_t<double>& input_data, const int& window) {
    auto buf_in = input_data.request();
    const double* ptr_in = static_cast<double*>(buf_in.ptr);
    const ssize_t size = buf_in.size;

    auto result = py::array_t<double>(size);
    double* ptr_out = static_cast<double*>(result.request().ptr);

    // Initial state: Fill everything with NaN 
    std::fill(ptr_out, ptr_out + size, std::numeric_limits<double>::quiet_NaN());

    if (size < window) {
        return result;
    }

    // Initial window sum
    double current_sum = std::accumulate(ptr_in, ptr_in + window, 0.0);
    ptr_out[window - 1] = current_sum / window;

    // Sliding window logic
    for (ssize_t i = window; i < size; ++i) {
        current_sum += ptr_in[i] - ptr_in[i - window];
        ptr_out[i] = current_sum / window;
    }

    return result;
}

py::array_t<double> calculate_ema_cpp(const py::array_t<double>& input_data, const int& window) {
    auto buf_in = input_data.request();
    const double* ptr_in = static_cast<double*>(buf_in.ptr);
    const ssize_t size = buf_in.size;

    auto result = py::array_t<double>(size);
    double* ptr_out = static_cast<double*>(result.request().ptr);

    if (size == 0) {
        return result;
    }

    const double alpha = 2.0 / (window + 1.0);
    const double beta = 1.0 - alpha;
    
    ptr_out[0] = ptr_in[0];

    // While EMA is inherently serial, we keep it clean. 
    // std::accumulate with a custom lambda could be used, but for a 
    // simple state-dependent recurrence, a clean loop is idiomatic.
    for (ssize_t i = 1; i < size; ++i) {
        ptr_out[i] = (ptr_in[i] * alpha) + (ptr_out[i - 1] * beta);
    }

    return result;
}


py::array_t<double> calculate_rsi_cpp(const py::array_t<double>& input_data, const int& window) {
    auto buf_in = input_data.request();
    const double* ptr_in = static_cast<double*>(buf_in.ptr);
    const ssize_t size = buf_in.size;

    // RAII allocation via pybind11 (which internally manages the buffer)
    auto result = py::array_t<double>(size);
    double* ptr_out = static_cast<double*>(result.request().ptr);

    // Initialise output with NaN
    if (size <= window) {
        std::fill(ptr_out, ptr_out + size, std::numeric_limits<double>::quiet_NaN());
        return result;
    }
    std::fill(ptr_out, ptr_out + window, std::numeric_limits<double>::quiet_NaN());

    double avg_gain = 0.0;
    double avg_loss = 0.0;
    const double alpha = 1.0 / window; 

    // Calculate first average gain and loss
    for (ssize_t i = 1; i <= window; ++i) {
        const double diff = ptr_in[i] - ptr_in[i - 1];
        if (diff >= 0) {
            avg_gain += diff;
        }
        else {
            avg_loss -= diff;
        }
    }
    avg_gain /= window;
    avg_loss /= window;

    // Lambda to compute RSI
    auto compute_rsi = [](double gain, double loss) -> double {
        if (loss == 0) {
            return 100.0;
        }
        double rs = gain / loss;
        return 100.0 - (100.0 / (1.0 + rs));
    };

    ptr_out[window] = compute_rsi(avg_gain, avg_loss);

    // Iterative computation (Dynamic Programming approach)
    for (ssize_t i = window + 1; i < size; ++i) {
        const double diff = ptr_in[i] - ptr_in[i - 1];
        const double gain = std::max(0.0, diff);
        const double loss = std::max(0.0, -diff);

        avg_gain = (gain * alpha) + (avg_gain * (1.0 - alpha));
        avg_loss = (loss * alpha) + (avg_loss * (1.0 - alpha));

        ptr_out[i] = compute_rsi(avg_gain, avg_loss);
    }

    return result;
}


/**
 * Calculates Bollinger Bands using a single-pass variance algorithm.
 * Returns a tuple: <Upper Band, Middle Band (SMA), Lower Band>
 */
std::tuple<py::array_t<double>, py::array_t<double>, py::array_t<double>> 
calculate_bollinger_bands_cpp(const py::array_t<double>& input_data, const int& window, const double& k) {
    auto buf_in = input_data.request();
    const double* ptr_in = static_cast<double*>(buf_in.ptr);
    const ssize_t size = buf_in.size;

    auto upper_arr = py::array_t<double>(size);
    auto mid_arr = py::array_t<double>(size);
    auto lower_arr = py::array_t<double>(size);

    double* ptr_u = static_cast<double*>(upper_arr.request().ptr);
    double* ptr_m = static_cast<double*>(mid_arr.request().ptr);
    double* ptr_l = static_cast<double*>(lower_arr.request().ptr);

    // Initialisation with NaN
    const double nan = std::numeric_limits<double>::quiet_NaN();
    std::fill(ptr_u, ptr_u + size, nan);
    std::fill(ptr_m, ptr_m + size, nan);
    std::fill(ptr_l, ptr_l + size, nan);

    if (size < window) {
        return std::make_tuple(upper_arr, mid_arr, lower_arr);
    }

    double sum = 0.0;
    double sum_sq = 0.0;

    // Initial window
    for (ssize_t i = 0; i < window; ++i) {
        sum += ptr_in[i];
        sum_sq += ptr_in[i] * ptr_in[i];
    }

    auto compute_bands = [&](ssize_t idx, double current_sum, double current_sum_sq) {
        double mean = current_sum / window;
        // Variance formula: (sum_sq - (sum^2 / N)) / N
        double variance = (current_sum_sq - (current_sum * current_sum / window)) / window;
        double std_dev = std::sqrt(std::max(0.0, variance)); // std::max handles precision noise

        ptr_m[idx] = mean;
        ptr_u[idx] = mean + (k * std_dev);
        ptr_l[idx] = mean - (k * std_dev);
    };

    compute_bands(window - 1, sum, sum_sq);

    // Sliding window logic
    for (ssize_t i = window; i < size; ++i) {
        sum += ptr_in[i] - ptr_in[i - window];
        sum_sq += (ptr_in[i] * ptr_in[i]) - (ptr_in[i - window] * ptr_in[i - window]);
        compute_bands(i, sum, sum_sq);
    }

    return std::make_tuple(upper_arr, mid_arr, lower_arr);
}

/**
 * Calculates MACD: Fast EMA, Slow EMA, and the Signal Line.
 * Returns <MACD Line, Signal Line>
 */
std::tuple<py::array_t<double>, py::array_t<double>> 
calculate_macd_cpp(const py::array_t<double>& input_data, 
                   const int& fast_period, 
                   const int& slow_period, 
                   const int& signal_period) {
    
    auto buf_in = input_data.request();
    const double* ptr_in = static_cast<double*>(buf_in.ptr);
    const ssize_t size = buf_in.size;

    auto macd_line_arr = py::array_t<double>(size);
    auto signal_line_arr = py::array_t<double>(size);
    
    double* ptr_macd = static_cast<double*>(macd_line_arr.request().ptr);
    double* ptr_signal = static_cast<double*>(signal_line_arr.request().ptr);

    if (size == 0) return std::make_tuple(macd_line_arr, signal_line_arr);

    // Initialise with NaNs
    const double nan = std::numeric_limits<double>::quiet_NaN();
    std::fill(ptr_macd, ptr_macd + size, nan);
    std::fill(ptr_signal, ptr_signal + size, nan);

    // EMA Constants
    const double alpha_fast = 2.0 / (fast_period + 1.0);
    const double alpha_slow = 2.0 / (slow_period + 1.0);
    const double alpha_sig  = 2.0 / (signal_period + 1.0);

    // Intermediate states for the two EMAs
    double current_ema_fast = ptr_in[0];
    double current_ema_slow = ptr_in[0];
    
    ptr_macd[0] = current_ema_fast - current_ema_slow;

    // Step 1: Calculate MACD Line (Difference of two EMAs)
    for (ssize_t i = 1; i < size; ++i) {
        current_ema_fast = (ptr_in[i] * alpha_fast) + (current_ema_fast * (1.0 - alpha_fast));
        current_ema_slow = (ptr_in[i] * alpha_slow) + (current_ema_slow * (1.0 - alpha_slow));
        ptr_macd[i] = current_ema_fast - current_ema_slow;
    }

    // Step 2: Calculate Signal Line (EMA of the MACD Line)
    // We start the Signal EMA from the first MACD value
    ptr_signal[0] = ptr_macd[0];
    for (ssize_t i = 1; i < size; ++i) {
        ptr_signal[i] = (ptr_macd[i] * alpha_sig) + (ptr_signal[i - 1] * (1.0 - alpha_sig));
    }

    return std::make_tuple(macd_line_arr, signal_line_arr);
}

int check_signals_cpp(const py::array_t<double>& rsi, 
                      const py::array_t<double>& price,
                      const py::array_t<double>& bb_upper, 
                      const py::array_t<double>& bb_lower) {
    
    auto r = rsi.unchecked<1>();
    auto p = price.unchecked<1>();
    auto up = bb_upper.unchecked<1>();
    auto lo = bb_lower.unchecked<1>();
    
    size_t last = r.shape(0) - 1;

    // Buy if price below lower band
    if (r(last) < 30 && p(last) < lo(last)) {
        return 1; // BUY
    }
    // Sell if price above upper band
    else if (r(last) > 70 && p(last) > up(last)) {
        return -1; // SELL
    }
    
    return 0; // HOLD
}



PYBIND11_MODULE(trading_core, m) {
    m.def("calculate_sma", &calculate_sma_cpp, "High performance SMA calculation");
    m.def("calculate_ema", &calculate_ema_cpp, "High performance EMA calculation");
    m.def("calculate_rsi", &calculate_rsi_cpp, "High performance RSI calculation");
    m.def("calculate_macd", &calculate_macd_cpp, "High performance MACD calculation");
    m.def("calculate_bollinger_bands", &calculate_bollinger_bands_cpp, "BB calculation");
    m.def("check_signals", &check_signals_cpp, "Detect trading signals");
}