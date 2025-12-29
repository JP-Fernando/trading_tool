#include "utils/logger.h"
#include <pybind11/pybind11.h>
#include <iostream>

namespace py = pybind11;

LogCallback Logger::callback = nullptr;
std::mutex Logger::log_mutex;

void Logger::set_callback(LogCallback cb)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    callback = cb;
}

void Logger::log(LogLevel level, const std::string &msg)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (callback)
    {
        // Is Python interpreter on?
        if (Py_IsInitialized())
        {
            try
            {
                // Acquire Python's GIL
                py::gil_scoped_acquire acq;
                callback(level, msg);
            }
            catch (const py::error_already_set &e)
            {
                // Log Python error to avoid breaking C++
                std::cerr << "Logger error: " << e.what() << "\n";
            }
        }
    }
}