#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

/// @brief A thread pool implementation for parallel execution of tasks.
class ThreadPool {
public:
    /// @brief Constructor
    /// @param num_threads Number of threads in the pool.
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    /// @brief Destructor
    ~ThreadPool();

    // Avoid copy & move
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /// @brief Adds a task to the thread pool.
    /// @param task The task to be executed.
    void enqueue(std::function<void()> task);

    /// @brief Stop the pool.
    void stop();

private:
    /// @brief Worker loop for each thread.
    void worker_loop();

    std::vector<std::jthread> workers; // C++20: jthread joins automatically
    std::queue<std::function<void()>> tasks;
    
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop_flag;
};

#endif