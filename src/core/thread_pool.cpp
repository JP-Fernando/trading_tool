#include "core/thread_pool.h"

/// @brief Constructor
ThreadPool::ThreadPool(size_t num_threads) : stop_flag(false)
{
    workers.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers.emplace_back([this]
                             { this->worker_loop(); });
    }
}

/// @brief Destructor
ThreadPool::~ThreadPool()
{
    stop();
    // jthreads join here
}

/// @brief Adds a task to the thread pool.
void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

/// @brief Stop the pool.
void ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop_flag = true;
    }
    condition.notify_all();
}

/// @brief Worker loop for each thread.
void ThreadPool::worker_loop()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            condition.wait(lock, [this]
                           { return !tasks.empty() || stop_flag; });

            if (stop_flag && tasks.empty())
            {
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
        }

        // Non-blocking: execute task
        task();
    }
}