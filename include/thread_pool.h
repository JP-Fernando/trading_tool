#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
public:
    // Constructor 
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    // Destructor
    ~ThreadPool();

    // Avoid copy & move
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Append task
    void enqueue(std::function<void()> task);

    // Stop the pool
    void stop();

private:
    void worker_loop();

    std::vector<std::jthread> workers; // C++20: jthread joins automatically
    std::queue<std::function<void()>> tasks;
    
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop_flag;
};

#endif