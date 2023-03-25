#ifndef __LOGGER_H__
#define __LOGGER_H__

// #include "caja_infra/caja_node_base.h"
#include "caja_logger/caja_logger.h"
namespace infra {

class Logger {

 private:
    Logger();

    ~Logger();

 public:
    inline static caja_logger::CajaLogger &log() {
        return *s_logger;
    }

    static void init(caja_logger::CajaLogger &logger);

 private:

    static caja_logger::CajaLogger *s_logger;
};
};
#endif // __LOGGER_H__
