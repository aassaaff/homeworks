#include "controller_manager/controller_manager.h"

namespace controller_manager {
//======================================================================================================================//
//                                                   ControllerTimer                                                    //
//======================================================================================================================//
void ControllerTimer::timerStart() {
        m_start_time = std::chrono::system_clock::now();
        m_timer_running = true;
}

void ControllerTimer::timerStop() {
        m_end_time = std::chrono::system_clock::now();
        m_timer_running = false;
};

double ControllerTimer::elapsedMilliseconds() {
    std::chrono::time_point<std::chrono::system_clock> end_time;
    if(m_timer_running) 
        end_time = std::chrono::system_clock::now();
    else 
        end_time = m_end_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - m_start_time).count(); 
}

double ControllerTimer::setLoopHz(double hz) { 
    m_iteration_thresh = 1000 * 1 / hz;
    return m_iteration_thresh;
};

long int ControllerTimer::timerRateSleep() {
    long int sleep_time = m_iteration_thresh - elapsedMilliseconds();
    if(sleep_time < 0) {
        return sleep_time;
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        return sleep_time;
    }
}

//======================================================================================================================//
//                                                   ControllerManager                                                  //
//======================================================================================================================//

void ControllerManager::readNodeParameters() {
    infra::Logger::init(*logger_);
    YAML::Node config = YAML::LoadFile("/caja/controller_soft/controller_manager/config/controller_manager.yaml");
    std::string server_url = config["server_address"].as<std::string>();
    std::string modbus_ip = config["modbus_ip"].as<std::string>();
    int modbus_port = config["modbus_port"].as<int>();
    m_loop_hz = config["loop_hz"].as<int>();
    std::cout << "+ Params:" << std::endl;
    std::cout << "- server address:\t" << server_url << std::endl;
    std::cout << "- modbus ip:\t" << modbus_ip << std::endl;
    std::cout << "- modbus port:\t" << modbus_port << std::endl;
    std::cout << "- loop Hz:\t" << m_loop_hz << std::endl;
    infra::Logger::log().info("ControllerManager::readNodeParameters - loop Hz: {}", m_loop_hz);
    SafetyController::initParams(server_url, modbus_ip, modbus_port, logger_); 
}

bool ControllerManager::internalInit() {
    double main_loop_cycle_time_ms = setLoopHz(m_loop_hz);
    infra::Logger::log().info("ControllerManager::internalInit - setting loop rate to: {} Hz, m_iteration_thresh: {}", m_loop_hz, main_loop_cycle_time_ms);
    if(!initModbusMaster()) {
        infra::Logger::log().error("ControllerManager::internalInit - was not able to init modbus master");
        return false;
    } else if (!initModbusCommands()) {
        infra::Logger::log().error("ControllerManager::internalInit - was not able to init modbus commands");
        return false;
    } else if (!initBackendClient()) { 
        infra::Logger::log().error("ControllerManager::internalInit - was not able to init backend client");
        return false;
    } else {
        return true; 
    }
};

void ControllerManager::mainThreadLooper() {
    bool backend_notified = false;
    while(true) {
        timerStart();
        infra::Logger::log().info("ControllerManager::mainThreadLooper - ========================================================================================");
        infra::Logger::log().info("ControllerManager::mainThreadLooper - **STAGE 1** - READ FROM PLC");
        const bool robots_can_move = readPLC();
        infra::Logger::log().info("ControllerManager::mainThreadLooper - **STAGE 2** - HANDLE RESPONSES FROM BE AND PLC");
        const bool can_enter = SafetyController::handleResponses(backend_notified, robots_can_move);
        infra::Logger::log().info("ControllerManager::mainThreadLooper - **STAGE 3** - WRITE TO PLC");
        SafetyController::writePLC(can_enter);
        infra::Logger::log().info("ControllerManager::mainThreadLooper - **STAGE 4** - NOTIFY BE");
        backend_notified = SafetyController::notifyBackend(robots_can_move);
        infra::Logger::log().info("ControllerManager::mainThreadLooper - cycle ended ; sleeping until next cycle needs to start");
        if (timerRateSleep() < 0) {
            infra::Logger::log().error("ControllerManager::mainThreadLooper - call diff > 500 ms, above thresh, code is hanging somewhere");
        }    
    }
}
};

//========================================================================================================//
//                                                main()                                                  //
//========================================================================================================//
int main(int argc, char **argv) {
    controller_manager::ControllerManager controller_manager_handler;
    if (!controller_manager_handler.cajaInit()) {
        std::cout << "cajaInit() failed... check your code" << std::endl;
        return 1;
    }
    controller_manager_handler.mainThreadLooper(); //this should call an inhereted method
    exit(0);
}


/*
run:
read plc
handle be response
do something
write plc
send be request
*/