cmake_minimum_required(VERSION 3.5.0)
project(controller_manager VERSION 1.0.0 LANGUAGES CXX)
add_compile_options(-std=c++11)

set ( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} /usr/local/lib/cmake/) #needed for CPR
find_package ( CPR REQUIRED )
find_package(nlohmann_json 3.2.0 REQUIRED)
# find_package(Boost REQUIRED COMPONENTS system)

include_directories ( ${CPR_INCLUDE_DIRS} )
# include_directories(include ${Boost_INCLUDE_DIRS})
# add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
add_library(caja_backend_api src/caja_backend_api/caja_backend_api.cpp)
add_library(safety_controller src/controller_manager/safety_controller.cpp include/controller_manager/safety_controller.h)
include_directories(${PROJECT_SOURCE_DIR}/../commands/include/)
include_directories(${PROJECT_SOURCE_DIR}/../infra/include/)
include_directories(${PROJECT_SOURCE_DIR}/../caja_infra/include/)
# include_directories(${PROJECT_SOURCE_DIR}/../caja_logger/include/)
include_directories(${PROJECT_SOURCE_DIR}/include/)
target_link_libraries(safety_controller caja_backend_api)
target_link_libraries(safety_controller commands)
target_link_libraries(caja_backend_api ${CPR_LIBRARIES})
# target_link_libraries(safety_controller ${BOOST_LABRARIES})
# set ( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} /caja/3rdparty/3rd_parties/yaml-cpp-yaml-cpp-0.6.3/build) #needed for cpp-yaml
find_package(yaml-cpp REQUIRED)
add_executable(${PROJECT_NAME}_node src/controller_manager/controller_manager.cpp)
target_link_libraries(${PROJECT_NAME}_node
  # ${BOOST_LABRARIES}
  # ${CPR_LIBRARIES}
  # ${NLOHMANN_LIBRARIES}
  # commands
  # caja_infra
  # caja_backend_api
  yaml-cpp
  # infra
  safety_controller
)
