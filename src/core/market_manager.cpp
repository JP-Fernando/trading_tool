#include "core/market_manager.h"
#include "utils/logger.h"
#include <iostream>
#include <cstdio>

// --- AssetData Implementation ---

void AssetData::add_price(double price, size_t max_size)
{
    prices.push_back(price);
    if (prices.size() > max_size)
    {
        prices.erase(prices.begin());
    }
}

// --- MarketManager Implementation ---

/// @brief Constructor
MarketManager::MarketManager(size_t num_threads) : pool(num_threads) {}

/// @brief Update tick data for a given symbol.
void MarketManager::update_tick(const std::string &symbol, double price)
{
    // Dispatch: Python entrega el dato y se libera inmediatamente
    pool.enqueue([this, symbol, price]()
                 { this->process_symbol(symbol, price); });
}

/// @brief Check last price, for debugging.
double MarketManager::get_last_price(const std::string &symbol)
{
    std::shared_lock<std::shared_mutex> lock(data_mutex);
    if (market_data.find(symbol) != market_data.end())
    {
        return market_data.at(symbol).prices.back();
    }
    return 0.0;
}

/// @brief Execution in threads
void MarketManager::process_symbol(std::string symbol, double price)
{
    std::vector<double> prices_snapshot;

    // 1. Critical Section: Update shared state
    {
        std::unique_lock<std::shared_mutex> lock(data_mutex);
        AssetData &data = market_data[symbol];
        data.add_price(price);
        // Copy to local vector to process WITHOUT lock
        prices_snapshot = data.prices;
    }

    // 2. Heavy Lifting: Parallel execution (No GIL, No Locks)
    // We need at least enough candles for the slowest indicator (MACD 26)
    if (prices_snapshot.size() >= 26)
    {
        auto rsi_v = compute_rsi(prices_snapshot, 14);
        auto [up, mid, lo] = compute_bollinger_bands(prices_snapshot, 20, 2.0);

        int signal = compute_signals(rsi_v, prices_snapshot, up, lo);

        if (signal != 0)
        {
            std::string action = (signal == 1) ? "BUY" : "SELL";
            std::string msg = "Symbol: " + symbol +
                              " | Price: " + std::to_string(prices_snapshot.back()) +
                              " | Action: " + action;
            Logger::log(LogLevel::SIGNAL, msg);
        }
    }
}
