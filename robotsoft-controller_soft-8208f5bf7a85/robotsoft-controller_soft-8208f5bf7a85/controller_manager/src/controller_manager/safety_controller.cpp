#include "controller_manager/safety_controller.h"

namespace controller_manager {

SafetyController::SafetyController() : m_server(nullptr) {}

SafetyController::~SafetyController() {
    delete m_server;
    delete m_modbus_handler;
}

bool SafetyController::initParams(std::string server_url, std::string modbus_ip, int modbus_port, caja_logger::CajaLogger* logger) {
    infra::Logger::init(*logger);
    infra::Logger::log().info("SafetyController::initParams - initializing parameters...");
    m_server_url = server_url;
    m_modbus_ip = modbus_ip;
    m_modbus_port = modbus_port;
    infra::Logger::log().info("SafetyController::initParams - server address: {}", server_url);
    infra::Logger::log().info("SafetyController::initParams - modbus ip: {}", modbus_ip);
    infra::Logger::log().info("SafetyController::initParams - modbus port: {}", modbus_port);
    return true;
}

bool SafetyController::initModbusMaster() {
    m_modbus_handler = new modbus(m_modbus_ip, m_modbus_port); 
    if(!m_modbus_handler->modbus_connect()){
        infra::Logger::log().error("SafetyController::internalInit - was not able to connect to modbus slave device");
        return false;
    } else { return true; }
}

bool SafetyController::initBackendClient() {
    bool safety_device = true;
    m_server = new caja_backend_api::CajaBackendAPI(m_server_url, safety_device);
    if(!m_server)
        return false;
    return true;
}

bool SafetyController::initModbusCommands() {
    if (!m_write_single_reg.init(*this, *m_modbus_handler, "WriteSingleRegister")) {
        infra::Logger::log().error("SafetyController::initializePublishers - WriteSingleRegister init failed");
        return false;
    } else if (!m_read_single_holding_reg.init(*this, *m_modbus_handler, "ReadSingleHoldingRegister")) {
        infra::Logger::log().error("SafetyController::initializePublishers - ReadSingleHoldingRegister init failed");
        return false;
    } else { return true; }
};

void SafetyController::commandFailedImp(commands::MissionCommand &command) {
    infra::Logger::log().error("SafetyController::commandFailedImp - command {} failed", command.getName());
    // need to implement error handling here.
}

const bool SafetyController::readPLC() {
    m_read_single_holding_reg.setArgs(1113);
    addCommand(m_read_single_holding_reg);
    executeCommands();
    uint16_t resp = m_read_single_holding_reg.getResult();
    m_read_single_holding_reg.completeCmd();
    infra::Logger::log().info("SafetyController::readPLC - value from PLC: {}", resp);
    return canRobotsMove(resp);
}

void SafetyController::writePLC(const bool can_enter) {
    uint16_t msg = can_enter; //yair found a bug
    infra::Logger::log().info("SafetyController::writePLC - writing to PLC - CW_WH_ENTER: {}", msg);
    m_write_single_reg.setArgs(2099, msg);
    addCommand(m_write_single_reg);
    executeCommands();
    m_write_single_reg.completeCmd();
}

bool SafetyController::notifyBackend(const bool robots_can_move) {
    if(robots_can_move) {
        infra::Logger::log().info("SafetyController::notifyBackend - POST request sent: safety/warehouse/robots/movement?safeForRobotsToMove=true");
    } else {
        infra::Logger::log().info("SafetyController::notifyBackend - POST request sent: safety/warehouse/robots/movement?safeForRobotsToMove=false");
    }
    m_robots_can_move_future_resp = m_server->safetyRobotsCanMoveAsync(robots_can_move);
    infra::Logger::log().info("SafetyController::notifyBackend - GET request sent to 'safety/warehouse/enter'");
    m_can_enter_future_resp = m_server->safetyCanEnterAsync();
    return true;
}

const bool SafetyController::handleResponses(bool &be_notified, const bool robots_can_move) {
    infra::Logger::log().info("SafetyController::handleResponses - handling responses, robots_can_move: {}, be_notified: {}", robots_can_move, be_notified);
    robotsCanMoveBackend(be_notified); 
    return translateToPLCcode(canEnterWarehouse(be_notified), robots_can_move);//need to fill this in
}

void SafetyController::robotsCanMoveBackend(const bool be_notified){
    if (be_notified) {
        if (m_robots_can_move_future_resp.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            m_robots_can_move_resp = m_robots_can_move_future_resp.get();
            if (m_robots_can_move_resp.status_code != caja_backend_api::status_code_e::STATUS_ACCEPTED)
                infra::Logger::log().error("SafetyController::robotsCanMoveBackend - got bad response code from BE: {}", m_robots_can_move_resp.status_code);
            else
                infra::Logger::log().info("SafetyController::robotsCanMoveBackend - got good response code from BE: 202");
        } else {
            infra::Logger::log().error("SafetyController::robotsCanMoveBackend - request timed out, above 500 ms");
        }
    } else {
        infra::Logger::log().info("SafetyController::robotsCanMoveBackend - be_notified: {} - wasn't anticipating response", be_notified);
    }
}

const bool SafetyController::canEnterWarehouse(bool &be_notified){
    if (be_notified) {
        be_notified = false;
        if(m_can_enter_future_resp.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            m_can_enter_resp = m_can_enter_future_resp.get();
            if (m_can_enter_resp.status_code != caja_backend_api::status_code_e::STATUS_SUCCESS) {
                infra::Logger::log().error("SafetyController::canEnterWarehouse - got bad response code from BE: {}", m_can_enter_resp.status_code);
                return false;
            } else {
                infra::Logger::log().info("SafetyController::canEnterWarehouse - got good response code from BE: 202 ; can_enter_WH: {}", m_can_enter_resp.response);
                return lockDoors(m_can_enter_resp.response);
            }
        } else {
            infra::Logger::log().error("SafetyController::canEnterWarehouse - request timed out, above 500 ms");
            return false;
        }
    } else {
        infra::Logger::log().info("SafetyController::canEnterWarehouse - be_notified: {} - wasn't anticipating response", be_notified);
        return false;
    }
}

const bool SafetyController::canRobotsMove(uint16_t resp) {
    switch(resp) {
        case state_codes::DOORS_LOCKED_ROBOTS_STOPPED:
            infra::Logger::log().info("SafetyController::canRobotsMove - resp: {} -> state_code: DOORS_LOCKED_ROBOTS_STOPPED", resp);
            return false;
            break;
        case state_codes::DOORS_LOCKED_ROBOTS_ACTIVE:
            infra::Logger::log().info("SafetyController::canRobotsMove - resp: {} -> state_code: DOORS_LOCKED_ROBOTS_ACTIVE", resp);
            return true;
            break;
        case state_codes::DOORS_OPEN_ROBOTS_STOPPED:
            infra::Logger::log().info("SafetyController::canRobotsMove - resp: {} -> state_code: DOORS_OPEN_ROBOTS_STOPPED", resp);
            return false;
            break;
        default:
            infra::Logger::log().info("SafetyController::canRobotsMove - resp: {} -> no status code for such value, stopping robots!!", resp);
            return false;
            break;
    }
}

bool SafetyController::lockDoors(const std::string be_resp_can_enter) {
    if(be_resp_can_enter == "true") {
        infra::Logger::log().info("SafetyController::lockDoors - BE responded that doors should be OPEN, lockDoors returning: false");
        return false;
    } else {
        infra::Logger::log().info("SafetyController::lockDoors - BE responded that doors should be LOCKED, lockDoors returning: true");
        return true;
    }
        
}

uint16_t SafetyController::translateToPLCcode(const bool doors_locked, const bool robots_can_move) {
    infra::Logger::log().info("SafetyController::translateToPLCcode - doors_locked: {}, robots_can_move: {}", doors_locked, robots_can_move);
    if(doors_locked && robots_can_move) {
        infra::Logger::log().info("SafetyController::translateToPLCcode - returning plc_codes::CANNOT_ENTER (value:{})", plc_codes::CANNOT_ENTER);
        return plc_codes::CANNOT_ENTER;
    } else if(!robots_can_move) {
        infra::Logger::log().info("SafetyController::translateToPLCcode - returning plc_codes::CAN_ENTER (value:{})", plc_codes::CAN_ENTER);
        return plc_codes::CAN_ENTER;
    } else {//PRTOECTION - SHOULD NOT BE POSSIBLE TO GET HERE.
        infra::Logger::log().info("SafetyController::translateToPLCcode - this shouldn't happen!! returning plc_codes::CANNOT_ENTER (value:{})", plc_codes::CANNOT_ENTER);
        return plc_codes::CANNOT_ENTER;
    }
}

std::string SafetyController::robotsCanMoveString(const bool robots_can_move) {
    nlohmann::json robots_can_move_string;
    bool status = robots_can_move ? true : false;
    robots_can_move_string["status"] = status;
    return robots_can_move_string.dump();
}

}

