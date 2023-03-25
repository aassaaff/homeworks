#include "caja_infra/caja_node_base.h"

namespace caja_infra {


CajaNodeBase::CajaNodeBase() {

}

bool CajaNodeBase::initLogger(const std::string& nodeName) {
    int maxLogFiles = 10;
    bool flushOnLogLevel = true;
    logger_ = new caja_logger::CajaLogger(nodeName, caja_logger::CajaLogger::Verbosity::INFO, maxLogFiles, flushOnLogLevel);
    logger_->info("CajaNodeBase::init - logger created");
    return true;
}

bool CajaNodeBase::cajaInit() {
    try {
        int maxLogFiles;
        bool flushOnLogLevel;
        // logger_ = new caja_logger::CajaLogger(nodeName(), caja_logger::CajaLogger::Verbosity::INFO, maxLogFiles, flushOnLogLevel);
        initLogger(nodeName());
        logger_->info("CajaNodeBase::init - logger created");
        setLogLevel();
        readParameters();
        logger_->info("init - calling internalInit");
        bool internalInitResponse = internalInit();
        std::cout << "internalInitResponse: " << internalInitResponse << std::endl;
        initializeServices();
        initializePublishers();
        initializeSubscribers();
        bool initializeActionsResponse = initializeActions();
        return (internalInitResponse && initializeActionsResponse);
    } catch (...) {
        logger_->error("init - failed with exception: {}", boost::current_exception_diagnostic_information());
        return false;
    }
}

void CajaNodeBase::setLogLevel() {
    logger_->info("CajaNodeBase::setLogLevel - start");
    int logLevel = caja_logger::CajaLogger::Verbosity::INFO;
    logger_->info("setLogLevel - level: {}", logLevel);
    logger_->setLevel(logLevel);
}

void CajaNodeBase::readParameters() {
    readNodeParameters(); // let derived classes read specific parameters
}

void CajaNodeBase::publishError(const std::string &errorCode, bool isErrorOn, std::string errorMetaData) {
    //for future use.
}

CajaNodeBase::~CajaNodeBase() {
    delete logger_;
}

}
