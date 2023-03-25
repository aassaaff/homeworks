#include "infra/logger.h"

namespace infra {

caja_logger::CajaLogger *Logger::s_logger = nullptr;

Logger::Logger() = default;

Logger::~Logger() = default;

void Logger::init(caja_logger::CajaLogger &logger) {
    s_logger = &logger;
}

}



