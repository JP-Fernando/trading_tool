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

py::array_t<double> calculate_sma_cpp(const py::array_t<double>& input_data, const int& window);

py::array_t<double> calculate_ema_cpp(const py::array_t<double>& input_data, const int& window);

py::array_t<double> calculate_rsi_cpp(const py::array_t<double>& input_data, const int& window);

std::tuple<py::array_t<double>, py::array_t<double>, py::array_t<double>> 
calculate_bollinger_bands_cpp(const py::array_t<double>& input_data, const int& window, const double& k);

std::tuple<py::array_t<double>, py::array_t<double>> 
calculate_macd_cpp(const py::array_t<double>& input_data, const int& fast, const int& slow, const int& signal);

    int check_signals_cpp(const py::array_t<double>& rsi, 
    const py::array_t<double>& price,
    const py::array_t<double>& bb_upper, 
    const py::array_t<double>& bb_lower);
                   

#endif