cmake_minimum_required(VERSION 3.15)
project(caja_infra)

add_compile_options(-std=c++11)

add_library(caja_infra src/caja_node_base.cpp include/caja_infra/caja_node_base.h)

find_package(Boost REQUIRED COMPONENTS system)

include_directories(include ${Boost_INCLUDE_DIRS})
add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(caja_infra
        caja_logger
)

# target_include_directories(caja_infra
#     INTERFACE
#         ${PROJECT_SOURCE_DIR}/include/caja_infra
# )