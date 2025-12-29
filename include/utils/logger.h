#include <string>
#include <functional>
#include <mutex>

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR,
    SIGNAL
};

using LogCallback = std::function<void(LogLevel, const std::string &)>;

class Logger
{
public:
    static void set_callback(LogCallback cb);

    static void log(LogLevel level, const std::string &msg);

private:
    static LogCallback callback;
    static std::mutex log_mutex;
};

