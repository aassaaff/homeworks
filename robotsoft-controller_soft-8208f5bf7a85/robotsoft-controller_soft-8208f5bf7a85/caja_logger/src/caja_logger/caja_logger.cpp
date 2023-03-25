#include "caja_logger/caja_logger.h"

namespace caja_logger {
spdlog::level::level_enum CajaLogger::spdLevel(caja_logger::CajaLogger::Verbosity logLevel) {
    switch (logLevel) {
        case Verbosity::OFF:
            return spdlog::level::off;
        case Verbosity::TRACE:
            return spdlog::level::trace;
        case Verbosity::DEBUG:
            return spdlog::level::debug;
        case Verbosity::INFO:
            return spdlog::level::info;
        case Verbosity::WARN:
            return spdlog::level::warn;
        case Verbosity::ERROR:
            return spdlog::level::err;
        case Verbosity::FATAL:
            return spdlog::level::critical;
        default:
            return spdlog::level::info;
    }
}

CajaLogger::CajaLogger(const std::string &logger_name, Verbosity logLevel, size_t max_num_of_files, bool flushOnLogeLevel)
    : logger_name_(logger_name), max_num_of_files_(max_num_of_files), flush_on_log_level_(flushOnLogeLevel) {
    auto host_name = boost::asio::ip::host_name();
    boost::algorithm::to_lower(logger_name_);
    boost::algorithm::to_lower(host_name);
    filename_ = base_path_ + logger_name_ + "_" + host_name + ".log";
    logLevel_ = logLevel;
    spdlog::level::level_enum spdLogLevel = spdLevel(logLevel);
    spdlog::set_pattern("%E%e\t%n\t%t\t%l\t%v");
    logger_ = spdlog::rotating_logger_mt<spdlog::async_factory>(logger_name_, filename_, MAX_FILE_SIZE, max_num_of_files);
    if (flush_on_log_level_) {
        logger_->flush_on(spdLogLevel);
    }
    logger_->set_level(spdLogLevel);
    logger_->info("Starting logger session, log level: {} max_log_files: {}", levelStr(logger_->level()), max_num_of_files);
}

CajaLogger::CajaLogger(const std::string &logger_name, Verbosity logLevel) : CajaLogger(logger_name, logLevel, DEFAULT_MAX_NUM_OF_FILES) {}

CajaLogger::CajaLogger(const std::string &logger_name) : CajaLogger(logger_name, Verbosity::INFO) {}

CajaLogger::~CajaLogger() {
    logger_->info("DTOR - perform flush and drop");
    logger_->flush();
    spdlog::drop(logger_name_);
}

void CajaLogger::setLevel(caja_logger::CajaLogger::Verbosity logLevel) {
    spdlog::level::level_enum spdLogLevel = spdLevel(logLevel);
    if (logLevel_ != Verbosity::OFF) {
        logger_->info("setLevel - from: {} to: {}, flush_on_log_level:{}", levelStr(logLevel_), levelStr(logLevel), flush_on_log_level_);
        logger_->set_level(spdLogLevel);
    } else {
        logger_->set_level(spdLogLevel);
        logger_->info("setLevel - from: {} to: {}, flush_on_log_level:{}", levelStr(logLevel_), levelStr(logLevel), flush_on_log_level_);
    }
    logLevel_ = logLevel;
    if (flush_on_log_level_) {
        logger_->flush_on(spdLogLevel);
    }
}

void CajaLogger::setLevel(int level) {
    setLevel(CajaLogger::fromInt(level));
}

bool CajaLogger::isTrace() {
    return logLevel_ <= Verbosity::TRACE;
}

bool CajaLogger::isDebug() {
    return logLevel_ <= Verbosity::DEBUG;
}

bool CajaLogger::isFlushOnLogLevel() {
    return flush_on_log_level_;
}

void CajaLogger::flush() {
    logger_->info("flush");
    logger_->flush();
}

void CajaLogger::write(Verbosity level, const std::string &msg) {
    switch (level) {
        case Verbosity::TRACE:
            logger_->trace(msg);
            break;
        case Verbosity::DEBUG:
            logger_->debug(msg);
            break;
        case Verbosity::INFO:
            logger_->info(msg);
            break;
        case Verbosity::WARN:
            logger_->warn(msg);
            break;
        case Verbosity::ERROR:
            logger_->error(msg);
            break;
        case Verbosity::FATAL:
            logger_->critical(msg);
            break;
        default:
            break;
    }
}
}