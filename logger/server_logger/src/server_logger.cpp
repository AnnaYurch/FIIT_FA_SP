#include "../include/server_logger.h"
#include "../include/server_logger_builder.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <ctime>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

server_logger::server_logger(const std::string& dest,
                             const std::unordered_map<logger::severity, std::pair<std::string, bool>>& streams)
    : _destination(dest), _output_streams(streams) {}

int server_logger::inner_getpid() {
#ifdef _WIN32
    return ::_getpid();
#else
    return getpid();
#endif
}

server_logger::~server_logger() noexcept {
    // Destructor implementation, for now, it's empty
}

server_logger::server_logger(const server_logger& other)
    : _destination(other._destination), _output_streams(other._output_streams) {}

server_logger& server_logger::operator=(const server_logger& other) {
    if (this != &other) {
        _destination = other._destination;
        _output_streams = other._output_streams;
    }
    return *this;
}

server_logger::server_logger(server_logger&& other) noexcept
    : _destination(std::move(other._destination)),
      _output_streams(std::move(other._output_streams)) {}

server_logger& server_logger::operator=(server_logger&& other) noexcept {
    if (this != &other) {
        _destination = std::move(other._destination);
        _output_streams = std::move(other._output_streams);
    }
    return *this;
}

logger& server_logger::log(const std::string& message, logger::severity severity) & {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char time_str[100];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::gmtime(&now_time));

    std::string severity_str;
    switch (severity) {
        case logger::severity::trace: severity_str = "trace"; break;
        case logger::severity::debug: severity_str = "debug"; break;
        case logger::severity::information: severity_str = "information"; break;
        case logger::severity::warning: severity_str = "warning"; break;
        case logger::severity::error: severity_str = "error"; break;
        case logger::severity::critical: severity_str = "critical"; break;
    }

    std::string log_message = std::string(time_str) + " " + severity_str + ": " + message;

    for (const auto& [sev, stream_info] : _output_streams) {
        if (severity >= sev && stream_info.second) { // Check if this severity level should be logged
            if (stream_info.first == "console") {
                std::cout << log_message << std::endl;
            } else {
                std::ofstream log_file(stream_info.first, std::ios::app);
                if (log_file.is_open()) {
                    log_file << log_message << std::endl;
                }
            }
        }
    }

    return *this;
}
