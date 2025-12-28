#ifndef MARKET_MANAGER_H
#define MARKET_MANAGER_H

#include <unordered_map>
#include <string>
#include <vector>
#include <shared_mutex>
#include "thread_pool.h"
#include "indicators.h"

// Asset data: vector of prices
struct AssetData {
    std::vector<double> prices;
    void add_price(double price, size_t max_size = 200);
};

class MarketManager {
public:
    explicit MarketManager(size_t num_threads = 4);
    ~MarketManager() = default;

    // Update asynchronously
    void update_tick(const std::string& symbol, double price);

    // Check last price, for debugging
    double get_last_price(const std::string& symbol);

private:
    // Execution in threads
    void process_symbol(std::string symbol, double price);

    ThreadPool pool;
    std::unordered_map<std::string, AssetData> market_data;
    mutable std::shared_mutex data_mutex; // C++17: Reader-Writer lock
};

#endif