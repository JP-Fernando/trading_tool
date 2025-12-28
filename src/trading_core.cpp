#include "trading_core.h"
#include "market_manager.h"


py::array_t<double> calculate_sma_cpp(const py::array_t<double>& input_data, const int& window) {
    // 1. Convert input data to std::vector
    auto buf = input_data.request();
    std::vector<double> input_vec(static_cast<double*>(buf.ptr), 
                                  static_cast<double*>(buf.ptr) + buf.size);

    // 2. Call function
    std::vector<double> res_vec = compute_sma(input_vec, window);

    // 3. Convert result to py::array_t
    return py::cast(res_vec);
}


py::array_t<double> calculate_ema_cpp(const py::array_t<double>& input_data, const int& window) {
    // 1. Convert input data to std::vector
    auto buf = input_data.request();
    std::vector<double> input_vec(static_cast<double*>(buf.ptr), 
                                  static_cast<double*>(buf.ptr) + buf.size);

    // 2. Call function
    std::vector<double> res_vec = compute_ema(input_vec, window);

    // 3. Convert result to py::array_t
    return py::cast(res_vec);
}



py::array_t<double> calculate_rsi_cpp(const py::array_t<double>& input_data, const int& window) {
    // 1. Convert input_data to std::vector
    auto buf = input_data.request();
    std::vector<double> input_vec(static_cast<double*>(buf.ptr), 
                                  static_cast<double*>(buf.ptr) + buf.size);

    // 2. Call function
    std::vector<double> result_vec = compute_rsi(input_vec, window);

    // 3. Convert result to py::array_t
    return py::cast(result_vec);
}

/**
 * Calculates Bollinger Bands using a single-pass variance algorithm.
 * Returns a tuple: <Upper Band, Middle Band (SMA), Lower Band>
 */
std::tuple<py::array_t<double>, py::array_t<double>, py::array_t<double>> 
calculate_bollinger_bands_cpp(const py::array_t<double>& input_data, const int& window, const double& k) {
    // 1. Convert input data to std::vector
    auto buf_in = input_data.request();
    std::vector<double> input_vec(static_cast<double*>(buf_in.ptr), 
                                  static_cast<double*>(buf_in.ptr) + buf_in.size);

    // 2. Call function
    auto [upper_v, mid_v, lower_v] = compute_bollinger_bands(input_vec, window, k);

    // 3. Convert to tuple of py::array_t
    return std::make_tuple(
        py::cast(upper_v), 
        py::cast(mid_v), 
        py::cast(lower_v)
    );
}

/**
 * Calculates MACD: Fast EMA, Slow EMA, and the Signal Line.
 * Returns <MACD Line, Signal Line>
 */
std::tuple<py::array_t<double>, py::array_t<double>> 
calculate_macd_cpp(const py::array_t<double>& input_data, 
                   const int& fast, 
                   const int& slow, 
                   const int& signal) {
    // 1. Convert input data to std::vector
    auto buf = input_data.request();
    std::vector<double> input_vec(static_cast<double*>(buf.ptr), 
                                static_cast<double*>(buf.ptr) + buf.size);

    // 2. Call function
    auto [macd_vec, signal_vec] = compute_macd(input_vec, fast, slow, signal);

    // 3. Convert to tuple of py::array_t
    return std::make_tuple(
        py::cast(macd_vec),
        py::cast(signal_vec)
    );
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
    m.doc() = "Multithreaded trading core";

    m.def("calculate_sma", &calculate_sma_cpp, "High performance SMA calculation");
    m.def("calculate_ema", &calculate_ema_cpp, "High performance EMA calculation");
    m.def("calculate_rsi", &calculate_rsi_cpp, "High performance RSI calculation");
    m.def("calculate_macd", &calculate_macd_cpp, "High performance MACD calculation");
    m.def("calculate_bollinger_bands", &calculate_bollinger_bands_cpp, "BB calculation");
    m.def("check_signals", &check_signals_cpp, "Detect trading signals");

    py::class_<MarketManager>(m, "MarketManager")
        .def(py::init<size_t>(), py::arg("threads") = 4)
        .def("update_tick", &MarketManager::update_tick)
        .def("get_last_price", &MarketManager::get_last_price);     
}