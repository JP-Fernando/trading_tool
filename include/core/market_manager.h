#ifndef MARKET_MANAGER_H
#define MARKET_MANAGER_H

#include <unordered_map>
#include <string>
#include <vector>
#include <shared_mutex>
#include "core/thread_pool.h"
#include "core/indicators.h"

/// @brief Asset data: vector of prices
struct AssetData {
    std::vector<double> prices;
    void add_price(double price, size_t max_size = 200);
};

/// @brief Manages market data and processes updates in a thread-safe manner.
/// This class maintains a thread pool and internal state for multiple assets.
/// It dispatches incoming ticks to worker threads for asynchronous indicator 
/// calculation and signal detection.
class MarketManager {
public:
    /// @brief Constructor
    /// @param num_threads Number of threads in the thread pool. 
    explicit MarketManager(size_t num_threads = 4);
    
    /// @brief Destructor
    ~MarketManager() = default;

    /// @brief Update tick data for a given symbol.
    /// @param symbol Asset symbol as a string.
    /// @param price Latest price as a double.
    void update_tick(const std::string& symbol, double price);

    /// @brief Check last price, for debugging.
    /// @param symbol Asset symbol as a string.
    /// @return Last price as a double.
    double get_last_price(const std::string& symbol);

private:
    /// @brief Execution in threads
    /// @param symbol Asset symbol as a string.
    /// @param price Latest price as a double.
    void process_symbol(std::string symbol, double price);

    ThreadPool pool;
    std::unordered_map<std::string, AssetData> market_data;
    mutable std::shared_mutex data_mutex; // C++17: Reader-Writer lock
};

#endif