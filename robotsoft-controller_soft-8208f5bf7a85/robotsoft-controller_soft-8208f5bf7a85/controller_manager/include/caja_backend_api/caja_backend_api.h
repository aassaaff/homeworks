#ifndef CAJA_BACKEND_API_H
#define CAJA_BACKEND_API_H

#include <string>
#include <mutex>

#include "cpr/cpr.h"
#include "cpr/api.h"
#include <chrono>
#include <map>
#include <nlohmann/json.hpp>
#include <iostream>
#include <regex>
#include <unistd.h>

namespace caja_backend_api {

typedef enum status_codes {
    STATUS_SUCCESS = 200,
    STATUS_ACCEPTED = 202,
    STATUS_SHUTDOWN = 220,
    MACHINE_LOGIN_EXPIRED = 419,
    INTERNAL_SERVER_ERROR = 500,
    MACHINE_NEXT_MISSIONS_NOT_FOUND = 404,
    STATUS_ABORT_REQUEST = 400
} status_code_e;


struct CajaBackendResponse {
    int status_code;
    std::string response;
    double elapsed;
    
    // CajaBackendResponse& operator =(const CajaBackendResponse& another)
    // {
    //     status_code = another.status_code;
    //     response = another.response;
    //     elapsed = another.elapsed;
    //     return *this;
    // }

};

class CajaBackendAPI {
 public:
    CajaBackendAPI() = default;
    CajaBackendAPI(const std::string &url, const std::string &machineID);
    CajaBackendAPI(const std::string &url, bool safety_device);
    CajaBackendResponse login(const std::string &request_data, int sessionTimeout);
    CajaBackendResponse keepAlive(const std::string &request_data, int sessionTimeout);
    CajaBackendResponse refreshMap(const std::string &request_data, int sessionTimeout);
    CajaBackendResponse next(const std::string &request_data, int sessionTimeout);
    CajaBackendResponse refreshMission(const std::string &request_data, int sessionTimeout);
    std::future<caja_backend_api::CajaBackendResponse> keepAliveAsync(const std::string &request_data);
    std::future<caja_backend_api::CajaBackendResponse> safetyCanEnterAsync();
    std::future<caja_backend_api::CajaBackendResponse> safetyRobotsCanMoveAsync(const bool move, const std::string &request_data="");
    
    // void reset();
    static std::string statusCodeString(int statusCode);

 private:
    CajaBackendResponse post_request(const cpr::Url &url, const std::string &data, cpr::Timeout sessionTimeout);
    CajaBackendResponse get_request(const cpr::Url &url, const std::string &data, cpr::Timeout sessionTimeout);
    CajaBackendResponse put_request(const cpr::Url &url, const std::string &data, cpr::Timeout sessionTimeout);
    std::future<caja_backend_api::CajaBackendResponse> get_request_async(cpr::Url &url);
    std::future<caja_backend_api::CajaBackendResponse> post_request_async(cpr::Url &url, const std::string &data);
    cpr::Session session_;
    cpr::Url url_;
    cpr::Url login_url_;
    cpr::Url keep_alive_url_;
    cpr::Url refresh_map_url_;
    cpr::Url next_url_;
    cpr::Url refresh_mission_url_;
    cpr::Url safety_wh_enter_url_;
    cpr::Url safety_robots_cant_move_url_;
    cpr::Url safety_robots_can_move_url_;
    std::mutex lock_;
    void initialize_params(const std::string &url, const std::string &machineID);
    void initialize_safety_params(const std::string &url);
};
}
#endif      //CAJA_BACKEND_API_H
