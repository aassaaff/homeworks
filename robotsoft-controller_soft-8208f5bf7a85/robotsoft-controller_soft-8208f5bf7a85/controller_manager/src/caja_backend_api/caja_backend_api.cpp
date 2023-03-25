#include "caja_backend_api/caja_backend_api.h"

namespace caja_backend_api {

std::string CajaBackendAPI::statusCodeString(int statusCode) {
    switch (statusCode) {
        case STATUS_SUCCESS:
            return "STATUS_SUCCESS";
        case STATUS_SHUTDOWN:
            return "STATUS_SHUTDOWN";
        case MACHINE_LOGIN_EXPIRED:
            return "MACHINE_LOGIN_EXPIRED";
        case INTERNAL_SERVER_ERROR:
            return "INTERNAL_SERVER_ERROR";
        case MACHINE_NEXT_MISSIONS_NOT_FOUND:
            return "MACHINE_NEXT_MISSIONS_NOT_FOUND";
        default:
            return "unsupported status enum value: " + std::to_string(statusCode);
    }
}

CajaBackendAPI::CajaBackendAPI(const std::string &url, const std::string &machineID) {
    std::cout << "base url: " << url << " ; machineID: " << machineID << std::endl;
    std::cout << "logging in" << std::endl;

    initialize_params(url, machineID);
}

CajaBackendAPI::CajaBackendAPI(const std::string &url, bool safety_device) {
    if(safety_device) {
        std::cout << "Safety controller base url: " << url << std::endl;
        initialize_safety_params(url);
    } else {
        std::cout << "Safety device CajaBackendAPI CTOR used, but not a safety device ...exiting" << std::endl;
        exit(1);
    }      
}

void CajaBackendAPI::initialize_safety_params(const std::string &url) {
    std::cout << "initialize_params, url: " << url << std::endl;
    url_ = cpr::Url{url.c_str()};
    safety_wh_enter_url_ = cpr::Url(url_ + "safety/warehouse/enter");
    safety_robots_can_move_url_ = cpr::Url(url_ + "safety/warehouse/robots/movement?safeForRobotsToMove=true");
    safety_robots_cant_move_url_ = cpr::Url(url_ + "safety/warehouse/robots/movement?safeForRobotsToMove=false");
    session_.SetHeader(cpr::Header{{"Content-Type", "application/json"}});
    std::cout << "initialize_params - saftey URLs:\n\t1) GET: " << safety_wh_enter_url_ << "\n\t2) POST: " << safety_robots_can_move_url_ << "\n\t3) POST: " << safety_robots_cant_move_url_ << std::endl;
}

void CajaBackendAPI::initialize_params(const std::string &url, const std::string &machineID) {
    std::cout << "initialize_params, url: " << url << std::endl;
    std::regex e("\\{(.*)\\}");
    std::string url_new = std::regex_replace(url, e, machineID);
    url_ = cpr::Url{url_new.c_str()};
    login_url_ = cpr::Url(url_new + "/login");
    keep_alive_url_ = cpr::Url(url_new + "/keepAlive");
    refresh_map_url_ = cpr::Url(url_new + "/map/refresh");
    next_url_ = cpr::Url(url_new + "/missions/next");
    refresh_mission_url_ = cpr::Url(url_new + "/missions/refresh");
    session_.SetHeader(cpr::Header{{"Content-Type", "application/json"}});
    std::cout << "initialize_params: " << login_url_ << std::endl;
}

// not being used anywhere and has been redacted form cpr
// void CajaBackendAPI::reset() {
//     session_.Reset();
//     session_.SetHeader(cpr::Header{{"Content-Type", "application/json"}});
// }

CajaBackendResponse CajaBackendAPI::get_request(const cpr::Url &url, const std::string &data, cpr::Timeout sessionTimeout) {
    lock_.lock();
    session_.SetUrl(url);
    session_.SetBody(cpr::Body(data));
    session_.SetTimeout(sessionTimeout);
    auto r = session_.Get();
    CajaBackendResponse res;
    res.status_code = r.status_code;
    res.response = r.text;
    res.elapsed = r.elapsed;
    lock_.unlock();
    return res;
}
CajaBackendResponse CajaBackendAPI::put_request(const cpr::Url &url, const std::string &data, cpr::Timeout sessionTimeout) {
    lock_.lock();
    session_.SetUrl(url);
    session_.SetBody(cpr::Body(data));
    session_.SetTimeout(sessionTimeout);
    auto r = session_.Put();
    CajaBackendResponse res;
    res.status_code = r.status_code;
    res.response = r.text;
    res.elapsed = r.elapsed;
    lock_.unlock();
    return res;
}
CajaBackendResponse CajaBackendAPI::post_request(const cpr::Url &url, const std::string &data, cpr::Timeout sessionTimeout) {
    lock_.lock();
    session_.SetUrl(url);
    session_.SetBody(cpr::Body(data));
    session_.SetTimeout(sessionTimeout);
    auto r = session_.Post();
    CajaBackendResponse res;
    res.status_code = r.status_code;
    res.response = r.text;
    res.elapsed = r.elapsed;
    lock_.unlock();
    return res;
}

std::future<caja_backend_api::CajaBackendResponse> CajaBackendAPI::post_request_async(cpr::Url &url, const std::string &data) {
    auto future_resp = cpr::PostCallback([](cpr::Response r)-> caja_backend_api::CajaBackendResponse {
        CajaBackendResponse res;
        res.status_code = r.status_code;
        res.response = r.text;
        res.elapsed = r.elapsed;
        return res;
    }, url, cpr::Body(data), cpr::Timeout(2000), cpr::Header{{"Content-Type", "application/json"}});

    return future_resp;
}

std::future<caja_backend_api::CajaBackendResponse> CajaBackendAPI::get_request_async(cpr::Url &url) {
    auto future_resp = cpr::GetCallback([](cpr::Response r)-> caja_backend_api::CajaBackendResponse {
        CajaBackendResponse res;
        res.status_code = r.status_code;
        res.response = r.text;
        res.elapsed = r.elapsed;
        return res;
    }, url, cpr::Timeout(2000), cpr::Header{{"Content-Type", "application/json"}});

    return future_resp;
}

CajaBackendResponse CajaBackendAPI::login(const std::string &request_data, int sessionTimeout) {
    std::cout << "login: " << login_url_ << std::endl;
    return post_request(login_url_, request_data, cpr::Timeout(sessionTimeout));
}

CajaBackendResponse CajaBackendAPI::refreshMap(const std::string &request_data, int sessionTimeout) {
    return get_request(refresh_map_url_, request_data, cpr::Timeout(sessionTimeout));
}

CajaBackendResponse CajaBackendAPI::refreshMission(const std::string &request_data, int sessionTimeout) {
    return put_request(refresh_mission_url_, request_data, cpr::Timeout(sessionTimeout));
}

CajaBackendResponse CajaBackendAPI::keepAlive(const std::string &request_data, int sessionTimeout) {
    return post_request(keep_alive_url_, request_data, cpr::Timeout(sessionTimeout));
}
std::future<caja_backend_api::CajaBackendResponse> CajaBackendAPI::keepAliveAsync(const std::string &request_data) {
    return post_request_async(keep_alive_url_, request_data);
}
CajaBackendResponse CajaBackendAPI::next(const std::string &request_data, int sessionTimeout) {
    return put_request(next_url_, request_data, cpr::Timeout(sessionTimeout));
}
std::future<caja_backend_api::CajaBackendResponse> CajaBackendAPI::safetyCanEnterAsync() {
    return get_request_async(safety_wh_enter_url_);
}
std::future<caja_backend_api::CajaBackendResponse> CajaBackendAPI::safetyRobotsCanMoveAsync(const bool move, const std::string &request_data) {
    if (move) {
        return post_request_async(safety_robots_can_move_url_, request_data);  
    } else {
        return post_request_async(safety_robots_cant_move_url_, request_data);  
    }
}
}
