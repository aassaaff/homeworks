#ifndef CONTROLLER_MANAGER_H
#define CONTROLLER_MANAGER_H
#include "controller_manager/safety_controller.h"
#include <yaml-cpp/yaml.h>
#include <chrono>
#include <ctime>
#include <cmath>

namespace controller_manager {
    
//======================================================================================================================//
//                                                   ControllerTimer                                                    //
//======================================================================================================================//
class ControllerTimer {
public:
    ControllerTimer() {};
    virtual ~ControllerTimer() {};
    void timerStart();
    void timerStop();
    double elapsedMilliseconds();
    double elapsedSeconds() { return elapsedMilliseconds() / 1000.0; }

protected:
    virtual double setLoopHz(double hz);
    virtual long int timerRateSleep();

private:
    std::chrono::time_point<std::chrono::system_clock> m_start_time;
    std::chrono::time_point<std::chrono::system_clock> m_end_time;
    bool m_timer_running = false;
    double m_iteration_thresh;
};

//======================================================================================================================//
//                                                   ControllerManager                                                  //
//======================================================================================================================//
class ControllerManager : public caja_infra::CajaNodeBase, public ControllerTimer, public SafetyController {
 public:
    ControllerManager() = default;
    virtual ~ControllerManager() {};
    void mainThreadLooper();

 protected:
    virtual bool internalInit() override;

    // ------------------- //
    // inhereted abstarcts // 
    // ------------------- //
    virtual const std::string nodeName() override { return "controller_manager"; }

    // ---------------------- //
    //   inhereted overrides  // 
    // ---------------------- //
    virtual void readNodeParameters() override;
    
 private:
    int     m_loop_hz;
};



}


#endif //CONTROLLER_MANAGER_H