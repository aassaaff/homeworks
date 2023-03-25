#ifndef CAJA_NODE_BASE_H
#define CAJA_NODE_BASE_H

#include <boost/exception/diagnostic_information.hpp>
#include <caja_logger/caja_logger.h>


namespace caja_infra {

class CajaNodeBase {
 public:
    CajaNodeBase();

    virtual ~CajaNodeBase();
    bool cajaInit();
    bool initLogger(const std::string& nodeName);

 protected:
    caja_logger::CajaLogger *logger_;
    
    virtual bool internalInit() { return true; };
    virtual void readNodeParameters() {};
    virtual void initializePublishers() {};
    virtual void initializeServices() {};
    virtual bool initializeActions() { return true; };
    virtual void initializeSubscribers() {};
    virtual void activateLoggerImp() {};

    void publishError(const std::string &errorCode, bool isErrorOn = true, std::string errorMetaData = "");

    virtual const std::string nodeName() = 0;

 private:
    void setLogLevel();
    void readParameters();

};
}
#endif
