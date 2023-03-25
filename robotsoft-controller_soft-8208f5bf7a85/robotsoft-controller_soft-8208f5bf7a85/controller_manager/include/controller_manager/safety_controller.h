#ifndef SAFETY_CONTROLLER_H
#define SAFETY_CONTROLLER_H

#include "caja_backend_api/caja_backend_api.h"
#include "commands/modbus_commands.h"
#include "caja_infra/caja_node_base.h"
// #include "commands/commander.h"
// #include "caja_logger/caja_logger.h"

// move to enum
typedef enum state_codes {
    DOORS_LOCKED_ROBOTS_STOPPED = 0,
    DOORS_LOCKED_ROBOTS_ACTIVE = 1,
    DOORS_OPEN_ROBOTS_STOPPED = 2,
} state_codes_e;

typedef enum plc_codes {
    CANNOT_ENTER = 0,
    CAN_ENTER = 1,
} plc_codes_e;

namespace controller_manager{

class SafetyController : public commands::CommandRunner {
 public:
    SafetyController();
    virtual ~SafetyController();

 protected:
    virtual void commandFailedImp(commands::MissionCommand &command) override;
    virtual void executeEndedImp(bool success) override {}; // no use-case currently.
    virtual void progressUpdatedImp() override {}; // no use-case currently.

    virtual bool initParams(std::string server_url, std::string modbus_ip, int modbus_port, caja_logger::CajaLogger* logger);
    virtual bool initModbusMaster();
    virtual bool initBackendClient();
    virtual bool initModbusCommands();

    // ---------------------------- //
    //  protected member functions  // 
    // ---------------------------- //
    const bool readPLC();                     
    void writePLC(const bool can_enter);                    
    bool notifyBackend(const bool robots_can_move);   
    const bool handleResponses(bool &be_notified, const bool robots_can_move);                  
    void robotsCanMoveBackend(const bool be_notified);               
    const bool canEnterWarehouse(bool &be_notified);           
    const bool canRobotsMove(uint16_t resp);            
    bool lockDoors(const std::string be_resp_can_enter);
    uint16_t translateToPLCcode(const bool doors_locked, const bool robots_can_move);          
    std::string robotsCanMoveString(const bool robots_can_move);  

 private:
    // ---------------------------- //
    //  modbus objects & variables  // 
    // ---------------------------- //
    int                                 m_modbus_port;
    std::string                         m_modbus_ip;
    modbus*                             m_modbus_handler;
    commands::ReadSingleHoldingRegister m_read_single_holding_reg;
    commands::WriteSingleRegister       m_write_single_reg;
    uint16_t*                           m_holding_regs_resp;
    uint16_t                            m_plc_msg;

    // consider moving to backend module class and inheriting
    // ----------------------------- //
    //  backend objects & variables  // 
    // ----------------------------- //
    caja_backend_api::CajaBackendAPI*                   m_server;
    caja_backend_api::CajaBackendResponse               m_robots_can_move_resp;
    std::future<caja_backend_api::CajaBackendResponse>  m_robots_can_move_future_resp;    
    std::future<caja_backend_api::CajaBackendResponse>  m_can_enter_future_resp;
    caja_backend_api::CajaBackendResponse               m_can_enter_resp;
    caja_backend_api::status_code_e                     m_response_codes;
    std::string                                         m_server_url;

    // -------- //
    //  etc...  // 
    // -------- //
    bool    m_robots_can_move;
    bool    m_lock_doors;
};

}


#endif //SAFETY_CONTROLLER_H