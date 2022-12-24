#pragma once

#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <optional>
#include <mutex>
#include <thread>

#include <iostream>

using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

class Logger {
    auto GetTime() const {
        if (manual_ts_) {
            return *manual_ts_;
        }

        return std::chrono::system_clock::now();
    }

    auto GetTimeStamp() const {
        const auto now = GetTime();
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        return std::put_time(std::localtime(&t_c), "%F %T");
    }

    // Для имени файла возьмите дату с форматом "%Y_%m_%d"
    std::string GetFileTimeStamp() const {
        const auto now = GetTime();
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        char time_string[std::size("yyyy_mm_dd")];
        std::strftime(
            std::data(time_string),
            std::size(time_string),
            "%Y_%m_%d",
            std::localtime(&t_c)
        );
        return time_string;
    }

    Logger() = default;
    Logger(const Logger&) = delete;

  public:
    static Logger& GetInstance() {
        static Logger obj;
        return obj;
    }

    // Выведите в поток все аргументы.
    template<class... Ts>
    void Log(const Ts&... args) {
        const std::lock_guard<std::mutex> lock(m_);

        std::ofstream log_file(
            "/var/log/sample_log_" + GetFileTimeStamp() + ".log",
            std::ios::app
        );

        if (!log_file) {
            std::cerr << "Cannot open file" << std::endl;
            return;
        }

        log_file << GetTimeStamp() << ": ";
        ((log_file << args), ...);
        log_file << std::endl;
    }

    // Установите manual_ts_. Учтите, что эта операция может выполняться
    // параллельно с выводом в поток, вам нужно предусмотреть
    // синхронизацию.
    void SetTimestamp(std::chrono::system_clock::time_point ts) {
        const std::lock_guard<std::mutex> lock(m_);
        manual_ts_ = ts;
    }

  private:
    std::optional<std::chrono::system_clock::time_point> manual_ts_;
    std::mutex m_;
};
