#ifndef PROJECT_CAJA_LOGGER_H
#define PROJECT_CAJA_LOGGER_H

#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/asio/ip/host_name.hpp>

namespace caja_logger {
const size_t DEFAULT_MAX_NUM_OF_FILES = 10;

class CajaLogger {
 public:
    enum Verbosity {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        OFF
    };
    CajaLogger(const std::string &logger_name);
    CajaLogger(const std::string &logger_name, Verbosity logLevel);
    CajaLogger(const std::string &logger_name, Verbosity logLevel, size_t max_num_of_files, bool flushOnLogeLevel = true);
    CajaLogger(const CajaLogger &logger) {
        logger_name_ = logger.logger_name_;
        filename_ = logger.filename_;
        logger_ = logger.logger_;
        logLevel_ = logger.logLevel_;
        max_num_of_files_ = logger.max_num_of_files_;
    };
    ~CajaLogger();
    void setLevel(Verbosity level);
    void setLevel(int level);
    bool isDebug();
    bool isTrace();
    bool isFlushOnLogLevel();
    void flush();
    void write(Verbosity level, const std::string &msg);

    template<typename... Args>
    std::string format(const char *format_str, Args &&... args) {
        // Process arguments
        return fmt::format(format_str, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void trace(const char *format_str, Args &&... args) {
        if (isTrace()) {
            write(Verbosity::TRACE, format(format_str, std::forward<Args>(args)...));
        }
    }

    template<typename... Args>
    void debug(const char *format_str, Args &&... args) {
        if (isDebug()) {
            debug(format(format_str, std::forward<Args>(args)...));
        }
    }

    template<typename... Args>
    void info(const char *format_str, Args &&... args) {
        info(format(format_str, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void warn(const char *format_str, Args &&... args) {
        warn(format(format_str, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void error(const char *format_str, Args &&... args) {
        error(format(format_str, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void fatal(const char *format_str, Args &&... args) {
        fatal(format(format_str, std::forward<Args>(args)...));
    }

    void debug(const std::string &msg) {
        write(Verbosity::DEBUG, msg);
    }

    void info(const std::string &msg) {
        write(Verbosity::INFO, msg);
    }

    void warn(const std::string &msg) {
        write(Verbosity::WARN, msg);
    }

    void error(const std::string &msg) {
        write(Verbosity::ERROR, msg);
    }

    void fatal(const std::string &msg) {
        write(Verbosity::FATAL, msg);
    }

 private:
    std::string logger_name_;
    std::string filename_;
    std::shared_ptr<spdlog::logger> logger_, console_logger_;
    Verbosity logLevel_ = Verbosity::INFO;
    size_t max_num_of_files_;
    bool flush_on_log_level_;
    const std::string base_path_ = "/caja/logs/";
    const size_t MAX_FILE_SIZE = 1048576 * 5;

    std::string levelStr(int level) {
        switch (level) {
            case 0:
                return "TRACE";
            case 1:
                return "DEBUG";
            case 2:
                return "INFO";
            case 3:
                return "WARN";
            case 4:
                return "ERROR";
            case 5:
                return "FATAL";
            case 6:
                return "OFF";
            default:
                return "unsupported level";
        }
    }

    Verbosity fromInt(int level) {
        if (level < 0 || level > 6) {
            return Verbosity::INFO;
        } else {
            return static_cast<Verbosity>(level);
        }
    }

    static spdlog::level::level_enum spdLevel(Verbosity logLevel);
};
}

#endif  // PROJECT_CAJA_LOGGER_H
