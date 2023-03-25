cmake_minimum_required(VERSION 3.15)
project(infra)
add_compile_options(-std=c++11)

find_package(Boost REQUIRED COMPONENTS system)
include_directories(include ${Boost_INCLUDE_DIRS})

find_package(nlohmann_json 3.2.0 REQUIRED)

# find_package(caja_infra)
include_directories(${PROJECT_SOURCE_DIR}/../caja_infra/include/)

# find_package(caja_logger)
include_directories(${PROJECT_SOURCE_DIR}/../caja_logger/include/)

add_library(${PROJECT_NAME} src/logger.cpp include/infra/logger.h)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${CAJA_INFRA_INCLUDE_DIRS}
    caja_infra
)

target_link_libraries(${PROJECT_NAME}
    ${BOOST_LABRARIES}
    ${NLOHMANN_LIBRARIES}
    caja_infra
)